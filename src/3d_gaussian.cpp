#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <iostream>

// Vertex shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 mvp;
void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.3, 0.6, 0.9, 1.0); // Light blue color for the surface
}
)";

// Fragment shader for black edges
const char* edgeFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black color for the edges
}
)";

// Function to generate the grid of points and their z-values for the Gaussian
std::vector<float> generateGaussianSurface(int gridSize, float range) {
    std::vector<float> surfaceData;
    
    float step = 2.0f * range / (gridSize - 1);
    
    for (int i = 0; i < gridSize; ++i) {
        float x = -range + i * step;
        for (int j = 0; j < gridSize; ++j) {
            float y = -range + j * step;
            float z = std::exp(-(x * x + y * y));
            surfaceData.push_back(x);
            surfaceData.push_back(y);
            surfaceData.push_back(z);
        }
    }

    return surfaceData;
}

// Function to generate the indices for drawing the grid as a triangle mesh
std::vector<unsigned int> generateMeshIndices(int gridSize) {
    std::vector<unsigned int> indices;
    
    for (int i = 0; i < gridSize - 1; ++i) {
        for (int j = 0; j < gridSize - 1; ++j) {
            int topLeft = i * gridSize + j;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + gridSize;
            int bottomRight = bottomLeft + 1;
            
            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return indices;
}

// Function to generate the indices for drawing the grid edges (lines)
std::vector<unsigned int> generateEdgeIndices(int gridSize) {
    std::vector<unsigned int> edgeIndices;
    
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize - 1; ++j) {
            int start = i * gridSize + j;
            edgeIndices.push_back(start);
            edgeIndices.push_back(start + 1);
        }
    }

    for (int j = 0; j < gridSize; ++j) {
        for (int i = 0; i < gridSize - 1; ++i) {
            int start = i * gridSize + j;
            edgeIndices.push_back(start);
            edgeIndices.push_back(start + gridSize);
        }
    }

    return edgeIndices;
}

// Callback function for resizing the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Variables to store rotation angles
float pitch = 0.0f;
float yaw = 0.0f;

// Process all input
void processInput(GLFWwindow *window)
{
    // Handle keyboard input for rotation
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += 0.001f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= 0.001f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw -= 0.001f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw += 0.001f;
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Interactive 3D Gaussian Surface with Edges", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Build and compile the vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Build and compile the fragment shader for the surface
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Build and compile the fragment shader for the edges
    unsigned int edgeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(edgeFragmentShader, 1, &edgeFragmentShaderSource, NULL);
    glCompileShader(edgeFragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(edgeFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(edgeFragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders for the surface
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders for the edges
    unsigned int edgeShaderProgram = glCreateProgram();
    glAttachShader(edgeShaderProgram, vertexShader);
    glAttachShader(edgeShaderProgram, edgeFragmentShader);
    glLinkProgram(edgeShaderProgram);

    // Check for linking errors
    glGetProgramiv(edgeShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(edgeShaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Clean up shaders (they are linked into the program and are no longer needed)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(edgeFragmentShader);

    // Generate Gaussian surface data
    int gridSize = 50;
    float range = 2.0f;
    std::vector<float> surfaceData = generateGaussianSurface(gridSize, range);

    // Generate mesh indices for the surface and edges
    std::vector<unsigned int> meshIndices = generateMeshIndices(gridSize);
    std::vector<unsigned int> edgeIndices = generateEdgeIndices(gridSize);

    // Create a VAO and VBO to store the vertex data
    unsigned int VAO, VBO, EBO, edgeVAO, edgeEBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, surfaceData.size() * sizeof(float), &surfaceData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndices.size() * sizeof(unsigned int), &meshIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set up VAO for edges
    glGenVertexArrays(1, &edgeVAO);
    glGenBuffers(1, &edgeEBO);

    glBindVertexArray(edgeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Reuse the vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(unsigned int), &edgeIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader for the surface
        glUseProgram(shaderProgram);

        // Create transformations
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 0.0f, 1.0f));  // Yaw rotation
        model = glm::rotate(model, pitch, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch rotation
        glm::mat4 mvp = projection * view * model;

        // Pass the transformations to the shader
        unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        // Render the surface
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, 0);

        // Render the edges
        glUseProgram(edgeShaderProgram);
        mvpLoc = glGetUniformLocation(edgeShaderProgram, "mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(edgeVAO);
        glDrawElements(GL_LINES, edgeIndices.size(), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Deallocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &edgeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &edgeEBO);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
