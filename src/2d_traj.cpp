#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Vertex shader
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform float xPos;
    uniform float yPos;
    void main() {
        gl_Position = vec4(aPos.x + xPos, aPos.y + yPos, 0.0, 1.0);
    }
)glsl";

// Fragment shader
const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
    }
)glsl";

void updatePosition(float& x_position, float& y_position, float& x_velocity, float& y_velocity, float gravity, float drag_coefficient, float dt) {
    // Apply gravity
    y_velocity += gravity * dt;

    // Apply drag
    x_velocity += -drag_coefficient * x_velocity * dt;
    y_velocity += -drag_coefficient * y_velocity * dt;

    // Update positions
    x_position += x_velocity * dt;
    y_position += y_velocity * dt;
}

void checkCollision(float& x_position, float& y_position, float& x_velocity, float& y_velocity, float size) {
    if (x_position > 1.0f - size || x_position < -1.0f + size) {
        x_velocity = -x_velocity;
    }
    if (y_position > 1.0f - size || y_position < -1.0f + size) {
        y_velocity = -y_velocity;
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Bouncing Square with Drag", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    float vertices[] = {
        -0.05f, -0.05f,  // bottom left
        -0.05f,  0.05f,  // top left
         0.05f, -0.05f,  // bottom right
         0.05f,  0.05f   // top right
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float x_position = 0.0f;
    float y_position = 0.0f;
    float x_velocity = 0.04f;
    float y_velocity = 0.0f;
    float gravity = -0.004f;
    float drag_coefficient = 0.01f;
    float size = 0.05f;

    while (!glfwWindowShouldClose(window)) {
        float dt = 0.008f;

        updatePosition(x_position, y_position, x_velocity, y_velocity, gravity, drag_coefficient, dt);
        checkCollision(x_position, y_position, x_velocity, y_velocity, size);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1f(glGetUniformLocation(shaderProgram, "xPos"), x_position);
        glUniform1f(glGetUniformLocation(shaderProgram, "yPos"), y_position);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
