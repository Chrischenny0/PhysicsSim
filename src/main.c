#include "physics.h"

int main(int argc, char** argv){
    // Constants Initialization
    DSP_WIDTH = 1920;
    DSP_HEIGHT = 800;
    CIRCLE_RADIUS = 4;
    NUM_INSTANCES = 1500;
    GRAVITY_BOOL = 1;
    AOE = 15;

    // Setup window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(DSP_WIDTH, DSP_HEIGHT);
    glutInitWindowPosition(0, 100);
    glutCreateWindow("Physics Simulation");

    // glew import
    glewInit();

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
    glutMouseFunc(mouseActivation);
    glutMotionFunc(passiveMouse);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);

    glutMainLoop();

    // Dealloc
    dealloc();
    return 0;
}