#include <GL/glut.h>
#include <cmath>

// Initial angular momentum components
float Lx = 0.0f; 
float Ly = 1.0f; 
const float Lz = 1.0f; 
const float c = 1.0f; 
const float dt = 0.01f; 

float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;

// Length of the rod
const float rodLength = 3.0f;

// Fixed point height on the z-axis
const float fixedPointZ = 2.0f;

void update() {
    float newLx = Lx + dt * (-c * Ly);
    float newLy = Ly + dt * (c * Lx);
    Lx = newLx;
    Ly = newLy;

    // Update rotation angles
    angleX += Lx * dt;
    angleY += Ly * dt;
    angleZ += Lz * dt;
}

void drawAxes() {
    glBegin(GL_LINES);

    // X axis - Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(5.0f, 0.0f, 0.0f);

    // Y axis - Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);

    // Z axis - Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 5.0f);

    glEnd();
}

void drawGyroscope() {
    // Draw a simple 3D gyroscope (e.g., a cylinder with a sphere)
    glPushMatrix();

    // Translate the gyroscope to the fixed point
    glTranslatef(0.0f, 0.0f, fixedPointZ);

    // Apply the rotations
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f); // Precession around the Z-axis
    glRotatef(angleX, 1.0f, 0.0f, 0.0f); // Nutation
    glRotatef(angleY, 0.0f, 1.0f, 0.0f); // Nutation
    
    // Draw the gyroscope rod extending downwards
    glColor3f(0.8f, 0.1f, 0.1f);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.05, 0.05, rodLength, 32, 32);

    // Draw the gyroscope bob at the bottom of the rod
    glTranslatef(0.0f, 0.0f, rodLength);
    glutSolidSphere(0.1, 32, 32);

    gluDeleteQuadric(quad);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set the camera
    gluLookAt(8.0, 6.0, 10.0,  // Eye position
              0.0, 0.0, 0.0,  // Look at position
              0.0, 0.0, 1.0);  // Up vector

    // Draw the axes
    drawAxes();

    // Draw the gyroscope
    drawGyroscope();
    
    glutSwapBuffers();
}

void idle() {
    update();
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("3D Gyroscope Simulation");

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
