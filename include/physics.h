#ifndef PHYSICSSIM_PHYSICS_H
#define PHYSICSSIM_PHYSICS_H

#include "graphics.h"

// TODO: OPTIMIZEEE
// TODO: Remove magic numbers and base everything off the input constraints

// GLOBALS
extern int GRAVITY_BOOL;
extern float AOE; // In angstrom

void initStaticPhysics();

void initDynamicPhysics();

void physicsMainLoop(float deltaTime);

void screenResize();

void passiveMouse(int, int);

void mouseActivation(int, int, int, int);

void dealloc();

#endif //PHYSICSSIM_PHYSICS_H
