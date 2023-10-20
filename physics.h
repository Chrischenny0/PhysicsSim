#ifndef PHYSICSSIM_PHYSICS_H
#define PHYSICSSIM_PHYSICS_H

#include "graphics.h"

// TODO: Clean everything up
// TODO: Remove magic numbers and base everything off the input constraints
// TODO: Add user interaction to sim
// TODO: localized interaction ~ halfway done
// TODO: Fix update screen size


// Physics structs
typedef struct Vector{
    float xComp;
    float yComp;
} Vector;

typedef struct Circle{
    float *xPos;
    float *yPos;
    Vector vector;
} Circle;

typedef struct Bucket{
    Circle **circArray;
    int size;
    int capacity;
} Bucket;

typedef struct Buckets{
    Bucket *array;
    int size;
    int horizontal;
    int vertical;

} Buckets;


void initStaticPhysics();

void initDynamicPhysics();

void physicsMainLoop(float deltaTime);

void screenResize();

void dealloc();

#endif //PHYSICSSIM_PHYSICS_H
