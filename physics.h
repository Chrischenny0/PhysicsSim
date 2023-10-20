#ifndef PHYSICSSIM_PHYSICS_H
#define PHYSICSSIM_PHYSICS_H

#include "graphics.h"

// TODO: Clean everything up
// TODO: Remove magic numbers and base everything off the input constraints
// TODO: Add user interaction to sim
// TODO: localized interaction ~ halfway done
// TODO: Fix update screen size

void initStaticPhysics();

void initDynamicPhysics();

void physicsMainLoop(float deltaTime);

void screenResize();

void dealloc();

#endif //PHYSICSSIM_PHYSICS_H
