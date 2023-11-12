#ifndef PHYSICSSIM_GRAPHICS_H
#define PHYSICSSIM_GRAPHICS_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


// Entry point for physics loop
extern void (*PHYSICS_LOOP)(float);

// Physics callback for resizing
extern void (*PHYSICS_RESIZE)();

// Graphics globals
extern int CIRCLE_RADIUS;   // Radius of each circle
extern int NUM_INSTANCES; // Num of circles to draw
extern float CIRCLE_VERTS[12];   // Vertices of the base sprite
extern float *INSTANCE_POS;      // Position of each instance (created and share by GPU)
extern float *VECTOR_COMP;


// Display height and width
extern int DSP_WIDTH;
extern int DSP_HEIGHT;


void initGL();

void display();

void resize(int newWidth, int newHeight);

void idle();

#endif //PHYSICSSIM_GRAPHICS_H
