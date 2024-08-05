#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <random>
#include <tuple>
#include <algorithm>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 1056
#define PADDING 3.0f // Increased padding around the edges in meters
#define SENSOR_MARGIN 0.5f // Minimum distance from sensors to the edge

// Define M_PI if it is not defined
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Global variables
const GLfloat sensorRadius = 0.075f; // Radius of the sensors (0.15m diameter)
const GLfloat meanFreePath = 7.0f; // Mean free path for scattering in meters
const GLfloat absorptionLength = 11.0f; // Mean free path for absorption in meters
const GLfloat absorptionProbability = 0.1f; // Probability of absorption at each scattering event

struct Photon {
    std::vector<std::pair<GLfloat, GLfloat>> path; // Store the path of the photon
    GLfloat angle; // Current angle of movement
    bool active; // Whether the photon is still moving
    Photon(GLfloat startX, GLfloat startY, GLfloat angle) : angle(angle), active(true) {
        path.emplace_back(startX, startY);
    }
};

const GLfloat emitterX = 12.0f;
const GLfloat emitterY = 17.0f;
const GLfloat photonSpeed = 1.0f; // Speed of the photon beam
std::vector<Photon> photons;

std::mt19937 rng;
std::uniform_real_distribution<GLfloat> angleDist(0.0f, 2.0f * M_PI);
std::exponential_distribution<GLfloat> scatteringDist(1.0f / meanFreePath);
std::exponential_distribution<GLfloat> absorptionDist(1.0f / absorptionLength);

std::vector<std::pair<GLfloat, GLfloat>> sensor_centers;

void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides);
void drawGridOfCircles(int N, GLfloat radius, GLint numberOfSides);
void drawBox();
void drawGridLines();
void drawEmitter(GLfloat x, GLfloat y);
void drawPhotonRay(const Photon& photon);
void drawScatterEffect(GLfloat x, GLfloat y, GLfloat angle);
void drawAbsorptionEffect(GLfloat x, GLfloat y);
std::tuple<bool, std::pair<GLfloat, GLfloat>> check_walls(GLfloat prev_x, GLfloat prev_y, GLfloat curr_x, GLfloat curr_y);
std::tuple<std::pair<GLfloat, GLfloat>, int> check_sensors(GLfloat prev_x, GLfloat prev_y, GLfloat curr_x, GLfloat curr_y, GLfloat r = sensorRadius);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(void)
{
    // Initialize random number generator
    rng.seed(std::random_device()());

    GLFWwindow *window;

    // Initialize the library
    if (!glfwInit())
    {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sensor Alignment", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set the framebuffer size callback to maintain aspect ratio
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT); // specifies the part of the window to which OpenGL will draw (in pixels), convert from normalized to pixels
    glMatrixMode(GL_PROJECTION); // projection matrix defines the properties of the camera that views the objects in the world coordinate frame. Here you typically set the zoom factor, aspect ratio and the near and far clipping planes
    glLoadIdentity(); // replace the current matrix with the identity matrix and starts us a fresh because matrix transforms such as glOrtho and glRotate cumulate, basically puts us at (0, 0, 0)
    glOrtho(-PADDING, 25.0f + PADDING, -PADDING, 33.0f + PADDING, 0.0f, 1.0f); // essentially set coordinate system to match real-world dimensions in meters with padding
    glMatrixMode(GL_MODELVIEW); // (default matrix mode) modelview matrix defines how your objects are transformed (meaning translation, rotation and scaling) in your world
    glLoadIdentity(); // same as above comment

    // Set background color to light gray
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // Initialize sensor centers
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            GLfloat spacingX = (25.0f - 2 * SENSOR_MARGIN) / 9;
            GLfloat spacingY = (33.0f - 2 * SENSOR_MARGIN) / 9;
            GLfloat x = SENSOR_MARGIN + j * spacingX;
            GLfloat y = SENSOR_MARGIN + i * spacingY;
            sensor_centers.emplace_back(x, y);
        }
    }

    // Set the initial photon
    photons.emplace_back(emitterX, emitterY, angleDist(rng));

    // Set the initial time
    double lastTime = glfwGetTime();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Calculate dt
        double currentTime = glfwGetTime();
        double dt = currentTime - lastTime;
        lastTime = currentTime;

        // Update the position of the current photon
        if (!photons.empty() && photons.back().active) {
            Photon& photon = photons.back();
            if (photon.active) {
                GLfloat prev_x = photon.path.back().first;
                GLfloat prev_y = photon.path.back().second;

                // Sample distances for scattering and absorption
                GLfloat samp_sca = scatteringDist(rng);
                GLfloat samp_abs = absorptionDist(rng);
                GLfloat samp_dist = std::min(samp_sca, samp_abs);

                GLfloat dx = samp_dist * std::cos(photon.angle);
                GLfloat dy = samp_dist * std::sin(photon.angle);
                GLfloat next_x = prev_x + dx;
                GLfloat next_y = prev_y + dy;

                auto wall_result = check_walls(prev_x, prev_y, next_x, next_y);
                bool hit_wall = std::get<0>(wall_result);
                std::pair<GLfloat, GLfloat> wall_intersection = std::get<1>(wall_result);

                auto sensor_result = check_sensors(prev_x, prev_y, next_x, next_y);
                std::pair<GLfloat, GLfloat> sensor_intersection = std::get<0>(sensor_result);
                int sensor_index = std::get<1>(sensor_result);

                if (hit_wall) {
                    photon.path.emplace_back(wall_intersection);
                    photon.active = false;
                    drawAbsorptionEffect(wall_intersection.first, wall_intersection.second);
                } else if (sensor_index >= 0) {
                    photon.path.emplace_back(sensor_intersection);
                    photon.active = false;
                    drawAbsorptionEffect(sensor_intersection.first, sensor_intersection.second);
                } else {
                    photon.path.emplace_back(next_x, next_y);
                    if (samp_dist == samp_abs) {
                        photon.active = false;
                        drawAbsorptionEffect(next_x, next_y);
                    } else {
                        drawScatterEffect(next_x, next_y, photon.angle);
                        photon.angle = angleDist(rng);
                    }
                }
            }
        } else {
            // Emit a new photon if the previous one is no longer active
            photons.emplace_back(emitterX, emitterY, angleDist(rng));
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the grid and box
        drawGridLines();
        drawBox();

        // Render sensors
        drawGridOfCircles(10, sensorRadius, 100); // 10x10 grid of sensors with radius 0.075m (diameter 0.15m)

        // Draw the emitter
        drawEmitter(emitterX, emitterY);

        // Draw the photon rays
        for (const auto& photon : photons) {
            drawPhotonRay(photon);
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void drawGridOfCircles(int N, GLfloat radius, GLint numberOfSides)
{
    GLfloat spacingX = (25.0f - 2 * SENSOR_MARGIN) / static_cast<GLfloat>(N - 1); // Spacing in meters
    GLfloat spacingY = (33.0f - 2 * SENSOR_MARGIN) / static_cast<GLfloat>(N - 1); // Spacing in meters

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            GLfloat x = SENSOR_MARGIN + j * spacingX;
            GLfloat y = SENSOR_MARGIN + i * spacingY;
            drawCircle(x, y, 0.0f, radius, numberOfSides);
        }
    }
}

void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides)
{
    int numberOfVertices = numberOfSides + 2;

    GLfloat twicePi = 2.0f * M_PI;

    // Use dynamic allocation to avoid non-constant array sizes
    GLfloat* circleVerticesX = new GLfloat[numberOfVertices];
    GLfloat* circleVerticesY = new GLfloat[numberOfVertices];
    GLfloat* circleVerticesZ = new GLfloat[numberOfVertices];

    circleVerticesX[0] = x;
    circleVerticesY[0] = y;
    circleVerticesZ[0] = z;

    for (int i = 1; i < numberOfVertices; i++)
    {
        circleVerticesX[i] = x + static_cast<GLfloat>(radius * std::cos(i * twicePi / numberOfSides));
        circleVerticesY[i] = y + static_cast<GLfloat>(radius * std::sin(i * twicePi / numberOfSides));
        circleVerticesZ[i] = z;
    }

    // Use dynamic allocation for allCircleVertices
    GLfloat* allCircleVertices = new GLfloat[numberOfVertices * 3];

    for (int i = 0; i < numberOfVertices; i++)
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[(i * 3) + 1] = circleVerticesY[i];
        allCircleVertices[(i * 3) + 2] = circleVerticesZ[i];
    }

    // Draw the outline
    glColor3f(0.6f, 0.6f, 0.6f); // Gray color for outline
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, allCircleVertices);
    glDrawArrays(GL_LINE_LOOP, 0, numberOfVertices);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Draw the filled circle
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for circle
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, allCircleVertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, numberOfVertices);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Clean up dynamically allocated memory
    delete[] circleVerticesX;
    delete[] circleVerticesY;
    delete[] circleVerticesZ;
    delete[] allCircleVertices;
}

void drawEmitter(GLfloat x, GLfloat y)
{
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the emitter
    GLfloat halfSize = 0.25f; // Half the size of the square

    glBegin(GL_QUADS);
    glVertex2f(x - halfSize, y - halfSize);
    glVertex2f(x + halfSize, y - halfSize);
    glVertex2f(x + halfSize, y + halfSize);
    glVertex2f(x - halfSize, y + halfSize);
    glEnd();
}

void drawPhotonRay(const Photon& photon)
{
    if (photon.path.size() < 2) return;

    glColor3f(0.7f, 0.7f, 0.1f); // Yellow color for the photon beam

    glLineWidth(2.0f); // Thicker line for the photon beam

    glBegin(GL_LINES);
    for (size_t i = 0; i < photon.path.size() - 1; ++i) {
        glVertex2f(photon.path[i].first, photon.path[i].second);
        glVertex2f(photon.path[i + 1].first, photon.path[i + 1].second);
    }
    glEnd();
}

void drawScatterEffect(GLfloat x, GLfloat y, GLfloat angle)
{
    glColor3f(0.0f, 1.0f, 0.0f); // Green color for scattering

    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x + 0.5f * std::cos(angle), y + 0.5f * std::sin(angle)); // Short green line indicating scattering
    glEnd();
}

void drawAbsorptionEffect(GLfloat x, GLfloat y)
{
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for absorption

    GLfloat halfSize = 0.25f; // Size of the absorption effect

    glBegin(GL_QUADS);
    glVertex2f(x - halfSize, y - halfSize);
    glVertex2f(x + halfSize, y - halfSize);
    glVertex2f(x + halfSize, y + halfSize);
    glVertex2f(x - halfSize, y + halfSize);
    glEnd();
}

void drawBox()
{
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for the box

    glLineWidth(2.0f); // Thicker line for the box

    glBegin(GL_LINE_LOOP);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(25.0f, 0.0f);
    glVertex2f(25.0f, 33.0f);
    glVertex2f(0.0f, 33.0f);
    glEnd();
}

std::tuple<bool, std::pair<GLfloat, GLfloat>> check_walls(GLfloat prev_x, GLfloat prev_y, GLfloat curr_x, GLfloat curr_y) {
    const GLfloat left_wall = 0.0f;
    const GLfloat right_wall = 25.0f;
    const GLfloat bottom_wall = 0.0f;
    const GLfloat top_wall = 33.0f;

    std::vector<std::pair<GLfloat, GLfloat>> intersects;

    if (curr_x < left_wall) {
        GLfloat t = (left_wall - prev_x) / (curr_x - prev_x);
        GLfloat y_intersect = prev_y + t * (curr_y - prev_y);
        if (0 <= t && t <= 1 && bottom_wall <= y_intersect && y_intersect <= top_wall) {
            intersects.emplace_back(left_wall, y_intersect);
        }
    }

    if (curr_x > right_wall) {
        GLfloat t = (right_wall - prev_x) / (curr_x - prev_x);
        GLfloat y_intersect = prev_y + t * (curr_y - prev_y);
        if (0 <= t && t <= 1 && bottom_wall <= y_intersect && y_intersect <= top_wall) {
            intersects.emplace_back(right_wall, y_intersect);
        }
    }

    if (curr_y < bottom_wall) {
        GLfloat t = (bottom_wall - prev_y) / (curr_y - prev_y);
        GLfloat x_intersect = prev_x + t * (curr_x - prev_x);
        if (0 <= t && t <= 1 && left_wall <= x_intersect && x_intersect <= right_wall) {
            intersects.emplace_back(x_intersect, bottom_wall);
        }
    }

    if (curr_y > top_wall) {
        GLfloat t = (top_wall - prev_y) / (curr_y - prev_y);
        GLfloat x_intersect = prev_x + t * (curr_x - prev_x);
        if (0 <= t && t <= 1 && left_wall <= x_intersect && x_intersect <= right_wall) {
            intersects.emplace_back(x_intersect, top_wall);
        }
    }

    if (!intersects.empty()) {
        return std::make_tuple(true, intersects.front());  // Return the first valid intersection
    }

    return std::make_tuple(false, std::make_pair(0.0f, 0.0f));
}

std::tuple<std::pair<GLfloat, GLfloat>, int> check_sensors(GLfloat prev_x, GLfloat prev_y, GLfloat curr_x, GLfloat curr_y, GLfloat r) {
    for (size_t i = 0; i < sensor_centers.size(); ++i) {
        GLfloat x_cent = sensor_centers[i].first;
        GLfloat y_cent = sensor_centers[i].second;

        GLfloat A = prev_x - x_cent;
        GLfloat B = curr_x - prev_x;
        GLfloat C = prev_y - y_cent;
        GLfloat D = curr_y - prev_y;

        GLfloat a = B * B + D * D;
        GLfloat b = 2 * (A * B + C * D);
        GLfloat c = A * A + C * C - r * r;

        GLfloat discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            continue;
        }

        GLfloat sqrt_discriminant = std::sqrt(discriminant);

        GLfloat t1 = (-b - sqrt_discriminant) / (2 * a);
        GLfloat t2 = (-b + sqrt_discriminant) / (2 * a);

        if ((0 <= t1 && t1 <= 1) || (0 <= t2 && t2 <= 1)) {
            return { {x_cent, y_cent}, static_cast<int>(i) };
        }
    }
    return { {0.0f, 0.0f}, -2 };
}

void drawGridLines()
{
    glColor4f(0.8f, 0.8f, 0.8f, 0.5f); // Light gray color for grid lines with lower alpha value

    glLineWidth(1.0f); // Thinner line for grid

    glBegin(GL_LINES);
    // Vertical grid lines
    for (int i = 0; i <= 25; ++i)
    {
        glVertex2f(static_cast<GLfloat>(i), 0.0f);
        glVertex2f(static_cast<GLfloat>(i), 33.0f);
    }
    // Horizontal grid lines
    for (int i = 0; i <= 33; ++i)
    {
        glVertex2f(0.0f, static_cast<GLfloat>(i));
        glVertex2f(25.0f, static_cast<GLfloat>(i));
    }
    glEnd();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Maintain aspect ratio
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate aspect ratio
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    float worldWidth = 25.0f + 2 * PADDING;
    float worldHeight = 33.0f + 2 * PADDING;

    if (aspectRatio > worldWidth / worldHeight)
    {
        float newWidth = worldHeight * aspectRatio;
        float halfWidthDiff = (newWidth - worldWidth) / 2.0f;
        glOrtho(-halfWidthDiff, worldWidth + halfWidthDiff, -PADDING, 33.0f + PADDING, 0.0f, 1.0f);
    }
    else
    {
        float newHeight = worldWidth / aspectRatio;
        float halfHeightDiff = (newHeight - worldHeight) / 2.0f;
        glOrtho(-PADDING, 25.0f + PADDING, -halfHeightDiff, worldHeight + halfHeightDiff, 0.0f, 1.0f);
    }

    glMatrixMode(GL_MODELVIEW);
}
