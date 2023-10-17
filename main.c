#include "graphics.h"

// TODO: Clean everything up
// TODO: Add user interaction to sim
// TODO: localized interaction

// Physics structs
typedef struct ball{
    float *position;
    vector vector;
} ball;

const vector GRAVITY_VEC = {0.0f, -0.5f};

// Physics globals

ball *balls;

void initStaticPhysics(){
    // Set size constraints to make the squares square
    for(int i = 0; i < 12; i += 2){
        baseVertices[i] = points[i] * radius * 2 / width;
        baseVertices[i + 1] = points[i + 1] * radius * 2 / height;
    }

    printf("distance: %f\n",(float) radius / width);
}

void initDynamicPhysics(){
    balls = calloc(numOfBalls, sizeof(ball));

    int length = ceil(sqrt(numOfBalls));

    float row = 1.0 - ((float) radius / height) * 2 - 0.01;
    float column = -1.0 + ((float) radius / width) * 2 + 0.01;

    float tmpColumn;

    printf("%f\n", row);
    printf("%f\n", column);

    float diffY = (row * 2) / length - 0.02;
    float diffX = (column * -2) / length;

    for(int i = 0; i < length; i++){
        tmpColumn = column;
        for(int k = 0; k < length; k++){
            if(numOfBalls < i * length + k){
                goto done;
            }
            sharePositions[(i * length * 2) + (k * 2)] = tmpColumn + 0.01 * i;
            sharePositions[(i * length * 2) + (k * 2) + 1] = row;

            tmpColumn += diffX;
        }
        row -= diffY;
    }
    done:

    for(int i = 0 ; i < numOfBalls; i++){
        balls[i].position = sharePositions + i * 2;
    }


    prevTime = glutGet(GLUT_ELAPSED_TIME);
}


void applyForce(vector *dest, vector src, float deltaTime){
    dest->xComp += src.xComp * deltaTime;
    dest->yComp += src.yComp * deltaTime;
}

void moveParticle(ball *ball, float deltaTime){
    ball->position[0] += ball->vector.xComp * deltaTime;
    ball->position[1] += ball->vector.yComp * deltaTime;
}


float forceCalculate(float distance){
    // Adjust from pixels to angstrom (atomic units)
    distance *= 0.1375f;

    if(distance >= 4){
        return 0;
    }
    return powf(4 / distance, 12);
}

float repellingForce(ball left, ball right){
    float leftX = (left.position[0] + 1) * (float) width / 2;
    float leftY = (left.position[1] + 1) * (float) height / 2;
    float rightX = (right.position[0] + 1) * (float) width / 2;
    float rightY = (right.position[1] + 1) * (float) height / 2;


    float distance = sqrtf(powf(rightX - leftX, 2) + powf(rightY - leftY, 2));

    return forceCalculate(distance);
}

void physics(float deltaTime){
    float xConversion = width / 2.0f;
    float yConversion = height / 2.0f;

    deltaTime /= 2;

    for(int l = 0; l < 2; l++){
        for(int i = 0; i < numOfBalls; i++){
            // Apply gravity to each
            applyForce(&balls[i].vector, GRAVITY_VEC, deltaTime);

            for(int k = i + 1; k < numOfBalls; k++){
                // Calculate the repel force between each ball TODO: Make this localized
                float force = repellingForce(balls[i], balls[k]);
                if(force != 0){
                    ball *leftBall = &balls[i];
                    ball *rightBall = &balls[k];

                    if(leftBall->position[0] > rightBall->position[0]){
                        leftBall = &balls[k];
                        rightBall = &balls[i];
                    }

                    float angle = atan2f(rightBall->position[1] - leftBall->position[1], rightBall->position[0] - leftBall->position[0]);

                    vector repelForce = {cosf(angle) * force, sinf(angle) * force};

                    applyForce(&rightBall->vector, repelForce, deltaTime);

                    repelForce.xComp *= -1;
                    repelForce.yComp *= -1;

                    applyForce(&leftBall->vector, repelForce, deltaTime);

                }
            }

            float absX = fabsf(balls[i].position[0]);
            float absY = fabsf(balls[i].position[1]);

            if((1.0f - absX) * xConversion  <= 12) {
                balls[i].vector.xComp *= -1;
            }
            if((1.0f - absY) * yConversion  <= 12) {
                balls[i].vector.yComp *= -1;
            }
        }

        for(int i = 0; i < numOfBalls; i++){
            if(l == 0){
                balls[i].vector.xComp *= 0.999;
                balls[i].vector.yComp *= 0.999;
            }
            moveParticle(&balls[i], deltaTime / 2);
        }
    }

}


int main(int argc, char** argv){
    // Setup window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Triangle Using OpenGL");

    // Set window attributes
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

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
    glutMainLoop();

    // FREEDOM
    free(balls);
    return 0;
}