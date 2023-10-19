#include "physics.h"

int main(int argc, char** argv){
    // Setup window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(DSP_WIDTH = 1920, DSP_HEIGHT = 800);
    glutInitWindowPosition(0, 100);
    glutCreateWindow("Triangle Using OpenGL");

    // Set window attributes
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);

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
    glutIdleFunc(idle);

    glutMainLoop();

    // Dealloc
    dealloc();
    return 0;
}