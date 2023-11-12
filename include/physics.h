#ifndef PHYSICSSIM_PHYSICS_H
#define PHYSICSSIM_PHYSICS_H

#include "graphics.h"

// TODO: OPTIMIZEEE
// TODO: Clean everything up
// TODO: Remove magic numbers and base everything off the input constraints
// TODO: Add user interaction to sim
// TODO: localized interaction ~ halfway done

// GLOBALS
extern int GRAVITY_BOOL;

void initStaticPhysics();

void initDynamicPhysics();

void physicsMainLoop(float deltaTime);

void screenResize();

void passiveMouse(int, int);

void mouseActivation(int, int, int, int);

void dealloc();

#endif //PHYSICSSIM_PHYSICS_H
