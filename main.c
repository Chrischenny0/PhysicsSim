#include "physics.h"


int main(int argc, char** argv){
    // Setup window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
    glutInitWindowSize(DSP_WIDTH = 800, DSP_HEIGHT = 800);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Triangle Using OpenGL");

    // Set window attributes
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // glew import
    glewInit();

    // Sim Initialize
    initStaticPhysics();

    // opengl init
    initGL();

    // Dynamic Sim
    initDynamicPhysics();

    // Set the display physics
    physicsFunc = &physics;

    // Set opengl running functions
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutMainLoop();

    // FREEDOM
    free(balls);
    return 0;
}