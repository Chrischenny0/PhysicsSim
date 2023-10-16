#include "graphics.h"

// TODO: Clean everything up
// TODO: write sim program

// Physics structs
typedef struct ball{
    float *position;
    vector vector;
} ball;

const vector GRAVITY_VEC = {0.0f, -0.01f};

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

    float row = 1.0 - ((float) radius / height) * 2;
    float column = -1.0 + ((float) radius / width) * 2;

    float tmpColumn;

    printf("%f\n", row);
    printf("%f\n", column);

    float diffY = (row * 2) / length;
    float diffX = (column * -2) / length;

    for(int i = 0; i < length; i++){
        tmpColumn = column;
        for(int k = 0; k < length; k++){
            if(numOfBalls < i * length + k){
                goto done;
            }
            sharePositions[(i * length * 2) + (k * 2)] = tmpColumn;
            sharePositions[(i * length * 2) + (k * 2) + 1] = row;

            tmpColumn += diffX;
        }
        row -= diffY;
    }
    done:

    for(int i = 0 ; i < numOfBalls; i++){
        balls[i].position = sharePositions + i * 2;
    }
    balls[0].vector.xComp = 0.01;
    balls[0].vector.yComp = -0.01;


    prevTime = glutGet(GLUT_ELAPSED_TIME);
}


void applyVector(vector *dest, vector src){
    dest->xComp += src.xComp;
    dest->yComp += src.yComp;
}

void moveParticle(ball *ball, float deltaTime){
    ball->position[0] += ball->vector.xComp * deltaTime;
    ball->position[1] += ball->vector.yComp * deltaTime;
}

// TODO: Make this integral 0
float forceCalculate(float distance){
    // Adjust from atomic units to pixels
    distance *= 0.1375f;

//    float distBetween = 5.0f;
//    float pushStr = 3;
//    float pullStr = 2.99;
//    float distBetween = 5.0f;
//    float pushStr = 2;
//    float pullStr = 1.5;
    float distBetween = 5.0f;
    float pushStr = 3;
    float pullStr = 1;
    float force = (powf(distBetween / distance, pushStr) - powf(distBetween / distance, pullStr));

    if(force > 1){
        force = 1;
    }
    if(force < 0){
        return 0;
    }
    return force;
}

float repellingForce(ball left, ball right){
    float leftX = (left.position[0] + 1) * (float) width / 2;
    float leftY = (left.position[1] + 1) * (float) height / 2;
    float rightX = (right.position[0] + 1) * (float) width / 2;
    float rightY = (right.position[1] + 1) * (float) height / 2;


    float distance = sqrtf(powf(rightX - leftX, 2) + powf(rightY - leftY, 2));

    float outForce;

    if(distance < 50){
        outForce = forceCalculate(distance);
    }
    else{
        outForce = 0;
    }

    return outForce;
}

void physics(float deltaTime){
    float xConversion = width / 2.0f;
    float yConversion = height / 2.0f;

    for(int i = 0; i < numOfBalls; i++){
        applyVector(&balls[i].vector, GRAVITY_VEC);

        for(int k = i + 1; k < numOfBalls; k++){
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

                applyVector(&rightBall->vector, repelForce);

                repelForce.xComp *= -1;
                repelForce.yComp *= -1;

                applyVector(&leftBall->vector, repelForce);

                balls[i].vector.xComp *= 0.992;
                balls[i].vector.yComp *= 0.992;
                balls[k].vector.xComp *= 0.992;
                balls[k].vector.yComp *= 0.992;
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
        moveParticle(&balls[i], deltaTime);
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