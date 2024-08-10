#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>

float angleX = 0.0f;
float angleY = 0.0f;
float cameraDistance = 5.0f;
float rotationSpeed = 0.0f;

void renderText(const std::string& text, float x, float y, void* font) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

// Vertex Shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment Shader source code for the cube faces
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.8, 0.3, 0.3, 1.0); // Red color for the cube faces
}
)";

// Fragment Shader source code for the edges
const char* edgeFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black color for the edges
}
)";

// Cube vertices
float vertices[] = {
    // positions       
    -0.5f, -0.5f, -0.5f,  // 0
     0.5f, -0.5f, -0.5f,  // 1
     0.5f,  0.5f, -0.5f,  // 2
    -0.5f,  0.5f, -0.5f,  // 3
    -0.5f, -0.5f,  0.5f,  // 4
     0.5f, -0.5f,  0.5f,  // 5
     0.5f,  0.5f,  0.5f,  // 6
    -0.5f,  0.5f,  0.5f   // 7
};

// Indices for drawing the cube faces
unsigned int faceIndices[] = {
    0, 1, 2, 2, 3, 0, // Bottom face
    4, 5, 6, 6, 7, 4, // Top face
    0, 1, 5, 5, 4, 0, // Front face
    2, 3, 7, 7, 6, 2, // Back face
    1, 2, 6, 6, 5, 1, // Right face
    0, 3, 7, 7, 4, 0  // Left face
};

// Indices for drawing the cube edges
unsigned int edgeIndices[] = {
    0, 1, 1, 2, 2, 3, 3, 0, // Bottom face edges
    4, 5, 5, 6, 6, 7, 7, 4, // Top face edges
    0, 4, 1, 5, 2, 6, 3, 7  // Vertical edges
};

unsigned int VBO, VAO, EBO_faces, EBO_edges;
unsigned int shaderProgram, edgeShaderProgram;

void compileShader(unsigned int shader, const char* shaderSource, const std::string& shaderType) {
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::" << shaderType << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void initShaders() {
    // Cube faces shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    compileShader(vertexShader, vertexShaderSource, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    compileShader(fragmentShader, fragmentShaderSource, "FRAGMENT");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Edge shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    compileShader(vertexShader, vertexShaderSource, "VERTEX");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    compileShader(fragmentShader, edgeFragmentShaderSource, "FRAGMENT");

    edgeShaderProgram = glCreateProgram();
    glAttachShader(edgeShaderProgram, vertexShader);
    glAttachShader(edgeShaderProgram, fragmentShader);
    glLinkProgram(edgeShaderProgram);

    glGetProgramiv(edgeShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(edgeShaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::EDGE_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void initBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO_faces);
    glGenBuffers(1, &EBO_edges);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Buffer for faces
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_faces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faceIndices), faceIndices, GL_STATIC_DRAW);

    // Buffer for edges
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edgeIndices), edgeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawCubeEdges() {
    glUseProgram(edgeShaderProgram);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glDrawElements(GL_LINES, sizeof(edgeIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

void drawCubeFaces() {
    glUseProgram(shaderProgram);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_faces);
    glDrawElements(GL_TRIANGLES, sizeof(faceIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

void display() {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);  // Light gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -cameraDistance));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    unsigned int modelLoc, viewLoc, projectionLoc;

    // Draw the cube faces
    glUseProgram(shaderProgram);
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    drawCubeFaces();

    // Draw the cube edges
    glUseProgram(edgeShaderProgram);
    modelLoc = glGetUniformLocation(edgeShaderProgram, "model");
    viewLoc = glGetUniformLocation(edgeShaderProgram, "view");
    projectionLoc = glGetUniformLocation(edgeShaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    drawCubeEdges();

    // Draw the rotation speed text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::string text = "Rotation Speed: " + std::to_string(rotationSpeed);
    renderText(text, 700, 50, GLUT_BITMAP_HELVETICA_18);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void idle() {
    angleX += rotationSpeed;
    angleY += rotationSpeed;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': cameraDistance -= 0.1f; break;
        case 's': cameraDistance += 0.1f; break;
        case 'a': angleY -= 5.0f; break;
        case 'd': angleY += 5.0f; break;
        case 'q': angleX -= 5.0f; break;
        case 'e': angleX += 5.0f; break;
        case '+': rotationSpeed += 0.1f; break;
        case '-': rotationSpeed -= 0.1f; break;
        case 'r': rotationSpeed = 0.0f; break; // Reset rotation speed
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Cube with Shaders");

    glewInit();
    glEnable(GL_DEPTH_TEST);

    initShaders();
    initBuffers();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO_faces);
    glDeleteBuffers(1, &EBO_edges);

    return 0;
}
