#include "physics.h"

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

// CONSTANTS
// Gravity
static const Vector GRAVITY_VEC = {0.0f, -0.5f};

// Each circle refers to its coordinates in the shared position space with the GPU
static Circle *CIRCLES;

// Buckets for localized updates
static Buckets BUCKETS;


static float points[12] = {
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f
};


//----BUCKET FUNCTIONS----//
// Initialize buckets in relation to size of the screen
static void allocBuckets();

// Add a ball to passed bucket
static void addBallToBucket(Bucket *inBucket, Circle *inBall);

// Load all buckets
static void loadBuckets();

// Check for interactions between molecules in adjacent buckets
static void checkBuckets(Bucket *main, Bucket *adj, float deltaTime);


//----PHYSICS FUNCTIONS----//
// Apply force to destination vector
static void applyForce(Vector *dest, Vector src, float deltaTime);

// Move the circle by its current vector
static void moveCircle(Circle *circle, float deltaTime);

// Calculate the repelling force between the two circles
static float repellingForce(const Circle *left, const Circle *right);


//----PHYSICS ENTRY FUNCTIONS----//
// Initialize the circle in the gpu
void initStaticPhysics() {
    // Set size constraints to make the squares square
    for(int i = 0; i < 12; i += 2){
        CIRCLE_VERTS[i] = points[i] * CIRCLE_RADIUS * 2 / DSP_WIDTH;
        CIRCLE_VERTS[i + 1] = points[i + 1] * CIRCLE_RADIUS * 2 / DSP_HEIGHT;
    }
}

// Initialize all instances of the base sprite with initial offsets
void initDynamicPhysics() { // TODO: Spacing better
    CIRCLES = calloc(NUM_INSTANCES, sizeof(Circle));

    int length = ceil(sqrt(NUM_INSTANCES));

    float row = 1.0 - ((float) CIRCLE_RADIUS / DSP_HEIGHT) * 2 - 0.01;
    float column = -1.0 + ((float) CIRCLE_RADIUS / DSP_WIDTH) * 2 + 0.01;

    float tmpColumn;

    float diffY = (row * 2) / length;
    float diffX = (column * -2) / length;

    for(int i = 0; i < length; i++){
        tmpColumn = column;
        for(int k = 0; k < length; k++){
            if(NUM_INSTANCES < i * length + k){
                goto done;
            }
            INSTANCE_POS[(i * length * 2) + (k * 2)] = tmpColumn;
            INSTANCE_POS[(i * length * 2) + (k * 2) + 1] = row;

            tmpColumn += diffX;
        }
        row -= diffY;
    }
    done:

    for(int i = 0 ; i < NUM_INSTANCES; i++){
        CIRCLES[i].xPos = INSTANCE_POS + (i * 2);
        CIRCLES[i].yPos = INSTANCE_POS + (i * 2) + 1;
    }

    allocBuckets();
}

// *** MAIN PHYSICS LOOP ***
void physicsMainLoop(float deltaTime) {
    float xConversion = DSP_WIDTH / 2.0f;
    float yConversion = DSP_HEIGHT / 2.0f;

    int stepSize = 8;

    deltaTime /= stepSize;


    for(int step = 0; step < stepSize; step++){
        loadBuckets();

        for(int i = 0; i < NUM_INSTANCES; i++){
            moveCircle(&CIRCLES[i], deltaTime / 2);

            applyForce(&CIRCLES[i].vector, GRAVITY_VEC, deltaTime);
        }

        for(int i = 0; i < BUCKETS.size; i++){

            Bucket *currBucket = &BUCKETS.array[i];

            for(int k = 0; k < currBucket->size; k++){
                for(int m = k + 1; m < currBucket->size; m++){

                    float force = repellingForce(currBucket->circArray[k], currBucket->circArray[m]);

                    if(force != 0){
                        Circle *leftBall = currBucket->circArray[k];
                        Circle *rightBall = currBucket->circArray[m];

                        if(*leftBall->xPos > *rightBall->xPos){
                            leftBall = currBucket->circArray[m];
                            rightBall = currBucket->circArray[k];
                        }

                        float angle = atan2f(*rightBall->yPos - *leftBall->yPos, *rightBall->xPos - *leftBall->xPos);

                        Vector repelForce = {cosf(angle) * force, sinf(angle) * force};

                        applyForce(&rightBall->vector, repelForce, deltaTime);

                        repelForce.xComp *= -1;
                        repelForce.yComp *= -1;

                        applyForce(&leftBall->vector, repelForce, deltaTime);
                    }
                }
            }

            if(i % BUCKETS.horizontal != 0 && i + BUCKETS.horizontal - 1 < BUCKETS.size){
                checkBuckets(currBucket, BUCKETS.array + i + BUCKETS.horizontal - 1, deltaTime);
            }

            if(i + BUCKETS.horizontal < BUCKETS.size){
                checkBuckets(currBucket, BUCKETS.array + i + BUCKETS.horizontal, deltaTime);
            }

            if(i + BUCKETS.horizontal + 1 < BUCKETS.size){
                checkBuckets(currBucket, BUCKETS.array + i + BUCKETS.horizontal + 1, deltaTime);
            }

            if(i + 1 < BUCKETS.size && i + 1 % BUCKETS.horizontal != 0){
                checkBuckets(currBucket, BUCKETS.array + i + 1, deltaTime);
            }
        }

        for(int i = 0; i < NUM_INSTANCES; i++){
            float absX = fabsf(*CIRCLES[i].xPos);
            float absY = fabsf(*CIRCLES[i].yPos);

            if((1.0f - absX) * xConversion  <= 8) {
                CIRCLES[i].vector.xComp *= -1;
            }
            if((1.0f - absY) * yConversion  <= 8) {
                CIRCLES[i].vector.yComp *= -1;
            }
            if(step == 0){
                CIRCLES[i].vector.xComp *= 0.99;
                CIRCLES[i].vector.yComp *= 0.99;
            }
            moveCircle(&CIRCLES[i], deltaTime / 2);
        }

    }
}

// Resize callback for updating the circle sizes in the gpu
void screenResize() {
    for(int i = 0; i < 12; i += 2){
        CIRCLE_VERTS[i] = points[i] * CIRCLE_RADIUS * 2 / DSP_WIDTH;
        CIRCLE_VERTS[i + 1] = points[i + 1] * CIRCLE_RADIUS * 2 / DSP_HEIGHT;
    }
}

// Deallocate all memory
void dealloc() {
    for(int i = 0; i < BUCKETS.size; i++){
        free(BUCKETS.array[i].circArray);
    }
    free(BUCKETS.array);
}


//----HELPER FUNCTIONS----//
static void allocBuckets() {
    if(BUCKETS.array != NULL){
        for(int i = 0; i < BUCKETS.size; i++){
            free(BUCKETS.array[i].circArray);
        }
        free(BUCKETS.array);
    }

    BUCKETS.horizontal = ceil(DSP_WIDTH / 58.0); // TODO: Radius
    BUCKETS.vertical = ceil(DSP_HEIGHT / 58.0); // TODO: Radius

    BUCKETS.size = BUCKETS.horizontal * BUCKETS.vertical;

    BUCKETS.array = calloc(BUCKETS.size, sizeof(Bucket));
}

static void loadBuckets() {
    for(int i = 0; i < BUCKETS.size; i++){
        BUCKETS.array[i].size = 0;
    }
    Bucket *inBucket;

    for(int i = 0; i < NUM_INSTANCES; i++){
        int xPos = (int) (((*CIRCLES[i].xPos + 1) / 2) * DSP_WIDTH) / 58; // TODO: Radius
        int yPos = (int) (((*CIRCLES[i].yPos + 1) / 2) * DSP_HEIGHT) / 58; // TODO: Radius


        if(xPos + yPos * BUCKETS.horizontal >= BUCKETS.size || xPos + yPos * BUCKETS.horizontal < 0){
            continue;
        }

        inBucket = BUCKETS.array + xPos + yPos * BUCKETS.horizontal;

        if(inBucket->circArray == NULL){
            inBucket->capacity = 8;
            inBucket->size = 0;
            inBucket->circArray = calloc(inBucket->capacity, sizeof(Circle*));
        }
        if(inBucket->size == inBucket->capacity){
            inBucket->circArray = realloc(inBucket->circArray, sizeof(Circle**) * (inBucket->capacity *= 2));
        }
        inBucket->circArray[inBucket->size++] = CIRCLES + i;
    }
}

static void applyForce(Vector *dest, Vector src, float deltaTime) {
    dest->xComp += src.xComp * deltaTime;
    dest->yComp += src.yComp * deltaTime;
}

static void moveCircle(Circle *circle, float deltaTime) {
    *circle->xPos += circle->vector.xComp * deltaTime;
    *circle->yPos += circle->vector.yComp * deltaTime;
}

static float repellingForce(const Circle *left, const Circle *right) {
    float leftX = (*left->xPos + 1) * (float) DSP_WIDTH / 2; // TODO: DRY
    float leftY = (*left->yPos + 1) * (float) DSP_HEIGHT / 2;
    float rightX = (*right->xPos + 1) * (float) DSP_WIDTH / 2;
    float rightY = (*right->yPos + 1) * (float) DSP_HEIGHT / 2;


    float distance = sqrtf(powf(rightX - leftX, 2) + powf(rightY - leftY, 2));  // TODO: DRY

    distance *= 0.1375f;

    if(distance >= 4){
        return 0;
    }

    if(distance < 2.5){
        distance = 2.5f;
    }

    float force = powf(4 / distance, 12);

    return force;
}

static void checkBuckets(Bucket *main, Bucket *adj, float deltaTime) {
    if(main->size == 0 || adj->size == 0){
        return;
    }
    for(int i = 0; i < main->size; i++){
        for(int k = 0; k < adj->size; k++){
            float force = repellingForce(main->circArray[i], adj->circArray[k]);

            if(force != 0){
                Circle *leftBall = main->circArray[i];
                Circle *rightBall = adj->circArray[k];

                if(*leftBall->xPos > *rightBall->yPos){
                    leftBall = adj->circArray[k];
                    rightBall = main->circArray[i];
                }

                float angle = atan2f(*rightBall->yPos - *leftBall->yPos, *rightBall->xPos - *leftBall->xPos);

                Vector repelForce = {cosf(angle) * force, sinf(angle) * force};

                applyForce(&rightBall->vector, repelForce, deltaTime);

                repelForce.xComp *= -1;
                repelForce.yComp *= -1;

                applyForce(&leftBall->vector, repelForce, deltaTime);
            }
        }
    }
}
