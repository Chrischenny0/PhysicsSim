# PhysicsSim

## Purpose
I've written this project to get practice in using modern OpenGL through writing shaders and directly communicating with the GPU. Drawing inspiration from [here]()

## What is it?
This project is meant to simulate the interactions between particles (in this case H<sub>2</sub>O). There are many different forces between molecules that give rise to the behavior that is observed so as most simulations do,
I've taken some artistic liberties to make it act closer to what you would expect. The one force I've modeled is called the Van der Waal force which are the interactions between dipoles of molecules. The main liberties taken are:
1) There is a slight attraction towards the end of the force curve that has been removed.
2) To model the loss of energy which would normally be through either molecules being ejected or loss of heat energy, there is a small fraction of energy removed every step.
3) Since each step is discrete instead of continuous, the force between molecules had to be capped to eliminate runaway interactions.

## Dependencies
- [FreeGlut]()
- [Glew]()
