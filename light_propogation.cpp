#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Constants
const float M_PI = 3.14159f;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float SENSOR_D = 4.0f; // Increased diameter for visibility
const float SENSOR_R = SENSOR_D / 2;
const int GRID_WIDTH = 2;
const int GRID_HEIGHT = 2;
const float DETECTOR_X = 25.0f;
const float DETECTOR_Y = 33.0f;
const int NUM_SEGMENTS = 24; // Number of segments for the circle

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void drawCircle(float cx, float cy, float r, int num_segments, bool printOnce);

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Circles", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set the callback function for window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Call the framebuffer_size_callback to set the initial aspect ratio
    framebuffer_size_callback(window, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Flag to print vertices only once
    bool printOnce = true;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the sensors
        for (int i = 0; i < GRID_WIDTH; ++i) {
            for (int j = 0; j < GRID_HEIGHT; ++j) {
                float x = SENSOR_D + i * (DETECTOR_X - 2 * SENSOR_D) / (GRID_WIDTH - 1);
                float y = SENSOR_D + j * (DETECTOR_Y - 2 * SENSOR_D) / (GRID_HEIGHT - 1);
                drawCircle(x, y, SENSOR_R, NUM_SEGMENTS, printOnce);
            }
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Disable further printing after the first frame
        printOnce = false;
    }

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    std::cout << "Framebuffer size callback: width = " << width << ", height = " << height << std::endl;

    // Adjust the viewport to the new window dimensions
    glViewport(0, 0, width, height);

    // Set the projection matrix mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate the aspect ratio of the window
    float aspect_ratio = (float)width / (float)height;
    std::cout << "Aspect ratio: " << aspect_ratio << std::endl;

    // Adjust the orthographic projection based on the aspect ratio
    if (aspect_ratio > 1.0f) {
        float ortho_width = DETECTOR_X * aspect_ratio;
        float ortho_height = DETECTOR_Y;
        glOrtho(0.0, ortho_width, 0.0, ortho_height, -1.0, 1.0);
    } else {
        float ortho_width = DETECTOR_X;
        float ortho_height = DETECTOR_Y / aspect_ratio;
        glOrtho(0.0, ortho_width, 0.0, ortho_height, -1.0, 1.0);
    }

    // Set the modelview matrix mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawCircle(float cx, float cy, float r, int num_segments, bool printOnce) {
    if (printOnce) {
        std::cout << "Drawing circle at (" << cx << ", " << cy << ") with radius " << r << std::endl;
    }

    // Draw the circle with filled triangles
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for the circle
    glVertex2f(cx, cy); // Center of the circle
    for (int i = 0; i <= num_segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments); // Angle
        float x = r * cosf(theta); // X coordinate
        float y = r * sinf(theta); // Y coordinate
        if (printOnce) {
            std::cout << "Vertex: (" << x + cx << ", " << y + cy << ")" << std::endl;
        }
        glVertex2f(x + cx, y + cy); // Vertex
    }
    glEnd();

    // Draw the edges of the triangles in a different color for debugging
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the edges
    for (int i = 0; i <= num_segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments); // Angle
        float x = r * cosf(theta); // X coordinate
        float y = r * sinf(theta); // Y coordinate
        glVertex2f(x + cx, y + cy); // Vertex
    }
    glEnd();
}
