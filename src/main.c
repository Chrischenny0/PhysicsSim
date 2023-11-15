#include "physics.h"

int main(int argc, char** argv){
    // Constants Initialization
    DSP_WIDTH = 1920;
    DSP_HEIGHT = 800;
    CIRCLE_RADIUS = 3;
    NUM_INSTANCES = 1500;
    GRAVITY_BOOL = 1;
    AOE = 10;

    // Setup window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(DSP_WIDTH, DSP_HEIGHT);
    glutInitWindowPosition(0, 100);
    glutCreateWindow("Physics Simulation");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

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