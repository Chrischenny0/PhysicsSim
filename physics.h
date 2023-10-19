#ifndef PHYSICSSIM_PHYSICS_H
#define PHYSICSSIM_PHYSICS_H

#include "graphics.h"

// TODO: Clean everything up
// TODO: Remove magic numbers and base everything off the input constraints
// TODO: Add user interaction to sim
// TODO: localized interaction ~ halfway done
// TODO: Fix update screen size


// Physics structs
typedef struct ball{
    float *xPos;
    float *yPos;
    vector vector;
} ball;

typedef struct Bucket{
    ball **balls;
    int size;
    int capacity;
} Bucket;

typedef struct Buckets{
    Bucket *array;
    int size;
    int horizontal;
    int vertical;

} Buckets;


// CONSTANTS
const vector GRAVITY_VEC = {0.0f, -0.5f};

ball *balls;

static Buckets buckets;

void initBuckets(){
    if(buckets.array != NULL){
        for(int i = 0; i < buckets.size; i++){
            free(buckets.array[i].balls);
        }
        free(buckets.array);
    }

    buckets.horizontal = ceil(DSP_WIDTH / 58.0);
    buckets.vertical = ceil(DSP_HEIGHT / 58.0);

    printf("HORIZONTAL: %d\n", buckets.horizontal);
    printf("VERTICAL: %d\n", buckets.vertical);

    buckets.size = buckets.horizontal * buckets.vertical;

    buckets.array = calloc(buckets.size, sizeof(Bucket));
}

void addBallToBucket(Bucket *inBucket, ball *inBall){
    if(inBucket->balls == NULL){
        inBucket->capacity = 8;
        inBucket->size = 0;
        inBucket->balls = calloc(inBucket->capacity, sizeof(ball*));
    }
    if(inBucket->size == inBucket->capacity){
        inBucket->balls = realloc(inBucket->balls, sizeof(ball**) * (inBucket->capacity *= 2));
    }
    inBucket->balls[inBucket->size++] = inBall;
}

void loadBuckets(){
    for(int i = 0; i < buckets.size; i++){
        buckets.array[i].size = 0;
    }

    for(int i = 0; i < numOfBalls; i++){

        int xPos = (int) (((*balls[i].xPos + 1) / 2) * DSP_WIDTH) / 58;
        int yPos = (int) (((*balls[i].yPos + 1) / 2) * DSP_HEIGHT) / 58;


        if(xPos + yPos * buckets.horizontal >= buckets.size || xPos + yPos * buckets.horizontal < 0){
            printf("skipped: %d\n", i);
            continue;
        }

        addBallToBucket(&buckets.array[xPos + yPos * buckets.horizontal], &balls[i]);

    }
}

void initStaticPhysics(){
    // Set size constraints to make the squares square
    for(int i = 0; i < 12; i += 2){
        baseVertices[i] = points[i] * radius * 2 / DSP_WIDTH;
        baseVertices[i + 1] = points[i + 1] * radius * 2 / DSP_HEIGHT;
    }
}

void initDynamicPhysics(){
    balls = calloc(numOfBalls, sizeof(ball));

    int length = ceil(sqrt(numOfBalls));

    float row = 1.0 - ((float) radius / DSP_HEIGHT) * 2 - 0.01;
    float column = -1.0 + ((float) radius / DSP_WIDTH) * 2 + 0.01;

    float tmpColumn;

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
        balls[i].xPos = sharePositions + (i * 2);
        balls[i].yPos = sharePositions + (i * 2) + 1;
    }


    initBuckets();

    prevTime = glutGet(GLUT_ELAPSED_TIME);
}


void applyForce(vector *dest, vector src, float deltaTime){
    dest->xComp += src.xComp * deltaTime;
    dest->yComp += src.yComp * deltaTime;
}

void moveParticle(ball *ball, float deltaTime){
    *ball->xPos += ball->vector.xComp * deltaTime;
    *ball->yPos += ball->vector.yComp * deltaTime;
}


float forceCalculate(float distance){
    // Adjust from pixels to angstrom (atomic units)
    distance *= 0.1375f;

    if(distance >= 4){
        return 0;
    }
//    if(distance < 3){
//        return 1;
//    }
    return powf(4 / distance, 12);
}

float repellingForce(const ball *left, const ball *right){
    float leftX = (*left->xPos + 1) * (float) DSP_WIDTH / 2;
    float leftY = (*left->yPos + 1) * (float) DSP_HEIGHT / 2;
    float rightX = (*right->xPos + 1) * (float) DSP_WIDTH / 2;
    float rightY = (*right->yPos + 1) * (float) DSP_HEIGHT / 2;


    float distance = sqrtf(powf(rightX - leftX, 2) + powf(rightY - leftY, 2));

    float force = forceCalculate(distance);

    return force;
}

void checkBuckets(Bucket *main, Bucket *adj, float deltaTime){
    for(int i = 0; i < main->size; i++){
        for(int k = 0; k < adj->size; k++){
            float force = repellingForce(main->balls[i], adj->balls[k]);

            if(force != 0){
                ball *leftBall = main->balls[i];
                ball *rightBall = adj->balls[k];

                if(*leftBall->xPos > *rightBall->yPos){
                    leftBall = adj->balls[k];
                    rightBall = main->balls[i];
                }

                float angle = atan2f(*rightBall->yPos - *leftBall->yPos, *rightBall->xPos - *leftBall->xPos);

                vector repelForce = {cosf(angle) * force, sinf(angle) * force};

                applyForce(&rightBall->vector, repelForce, deltaTime);

                repelForce.xComp *= -1;
                repelForce.yComp *= -1;

                applyForce(&leftBall->vector, repelForce, deltaTime);

            }
        }
    }
}

void physics(float deltaTime){
    float xConversion = DSP_WIDTH / 2.0f;
    float yConversion = DSP_HEIGHT / 2.0f;

    deltaTime /= 8;


    for(int step = 0; step < 8; step++){
        loadBuckets();

        for(int i = 0; i < numOfBalls; i++){
            applyForce(&balls[i].vector, GRAVITY_VEC, deltaTime);
        }

        for(int i = 0; i < buckets.size; i++){

            Bucket *currBucket = &buckets.array[i];

            for(int k = 0; k < currBucket->size; k++){
                for(int m = k + 1; m < currBucket->size; m++){

                    float force = repellingForce(currBucket->balls[k], currBucket->balls[m]);

                    if(force != 0){
                        ball *leftBall = currBucket->balls[k];
                        ball *rightBall = currBucket->balls[m];

                        if(*leftBall->xPos > *rightBall->xPos){
                            leftBall = currBucket->balls[m];
                            rightBall = currBucket->balls[k];
                        }

                        float angle = atan2f(*rightBall->yPos - *leftBall->yPos, *rightBall->xPos - *leftBall->xPos);

                        vector repelForce = {cosf(angle) * force, sinf(angle) * force};

                        applyForce(&rightBall->vector, repelForce, deltaTime);

                        repelForce.xComp *= -1;
                        repelForce.yComp *= -1;

                        applyForce(&leftBall->vector, repelForce, deltaTime);

                    }
                }
            }

            if(i % buckets.horizontal != 0 && i + buckets.horizontal - 1 < buckets.size){
                checkBuckets(currBucket, buckets.array + i + buckets.horizontal - 1, deltaTime);
            }

            if(i + buckets.horizontal < buckets.size){
                checkBuckets(currBucket, buckets.array + i + buckets.horizontal, deltaTime);
            }

            if(i + buckets.horizontal + 1 < buckets.size){
                checkBuckets(currBucket, buckets.array + i + buckets.horizontal + 1, deltaTime);
            }

            if(i + 1 < buckets.size && i + 1 % buckets.horizontal != 0){
                checkBuckets(currBucket, buckets.array + i + 1, deltaTime);
            }
        }

        for(int i = 0; i < numOfBalls; i++){
            float absX = fabsf(*balls[i].xPos);
            float absY = fabsf(*balls[i].yPos);

            if((1.0f - absX) * xConversion  <= 8) {
                balls[i].vector.xComp *= -1;
            }
            if((1.0f - absY) * yConversion  <= 8) {
                balls[i].vector.yComp *= -1;
            }
            if(step == 0){
                balls[i].vector.xComp *= 0.99;
                balls[i].vector.yComp *= 0.99;
            }
            moveParticle(&balls[i], deltaTime);
        }

    }


}

#endif //PHYSICSSIM_PHYSICS_H
