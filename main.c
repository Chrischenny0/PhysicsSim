#include "physics.h"

int main(int argc, char** argv){
    // Setup window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(DSP_WIDTH = 1920, DSP_HEIGHT = 800);
    glutInitWindowPosition(0, 100);
    glutCreateWindow("Triangle Using OpenGL");

    // Set window attributes

    // glew import
    glewInit();

    CIRCLE_RADIUS = 10;
    NUM_INSTANCES = 2000;

    // Sim Initialize
    initStaticPhysics();

    // opengl init
    initGL();

    // Dynamic Sim
    initDynamicPhysics();

    // Set the display physics
    PHYSICS_LOOP = &physicsMainLoop;
    PHYSICS_RESIZE = &screenResize;

    // Set opengl running functions
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);

    glutMainLoop();

    // Dealloc
    dealloc();
    return 0;
}