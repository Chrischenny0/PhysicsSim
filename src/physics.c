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
int GRAVITY_BOOL;
float AOE;

// Gravity
static Vector GRAVITY_VEC = {0.0f, -2.5f};

static Vector MOUSE_VEC = {-0.5f, -0.5f};

static int MOUSE_X;
static int MOUSE_Y;

long long hits = 0;
long long misses = 0;

// Each circle refers to its coordinates in the shared position space with the GPU
static Circle *CIRCLES;

// Buckets for localized updates
static Buckets BUCKETS;

static GLboolean MOUSE_ACTIVE;


static float points[12] = {
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f
};


//----BUCKET FUNCTIONS----// //TODO: Pass by reference
// Initialize buckets in relation to size of the screen
static void allocBuckets();

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
static inline float repellingForce(const Circle *left, const Circle *right);

static float distance(float x1, float y1, float x2, float y2);

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

//*** MAIN PHYSICS LOOP ***//
void physicsMainLoop(float deltaTime) {
    float xConversion = DSP_WIDTH / 2.0f;
    float yConversion = DSP_HEIGHT / 2.0f;

    int stepSize = 10;

    deltaTime /= stepSize;
    //
    // printf("HITS: %li\n", hits);
    // printf("MISSES: %li\n", misses);
    // printf("TOTAL: %li\n\n", misses + hits);

    hits = misses = 0;


    for(int step = 0; step < stepSize; step++){
        loadBuckets();

        for(int i = 0; i < NUM_INSTANCES; i++){
            moveCircle(&CIRCLES[i], deltaTime / 2);

            if(GRAVITY_BOOL == 1) applyForce(&CIRCLES[i].vector, GRAVITY_VEC, deltaTime);
            if(MOUSE_ACTIVE) applyForce(&CIRCLES[i].vector, MOUSE_VEC, deltaTime);
        }

        for(int i = 0; i < BUCKETS.size; i++){

            Bucket *currBucket = BUCKETS.array + i;

            if(currBucket->size == 0) continue;

            for(int k = 0; k < currBucket->size; k++){
                for(int m = k + 1; m < currBucket->size; m++){

                    if(fabsf(*currBucket->circArray[k]->xPos - *currBucket->circArray[m]->xPos) * DSP_WIDTH / 2 * 1.375 / CIRCLE_RADIUS > AOE) continue;

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

            if(i % BUCKETS.horizontal != 0 && i + BUCKETS.horizontal - 1 < BUCKETS.size && (BUCKETS.array + i + BUCKETS.horizontal - 1)->size != 0){
                checkBuckets(currBucket, BUCKETS.array + i + BUCKETS.horizontal - 1, deltaTime);
            }

            if(i + BUCKETS.horizontal < BUCKETS.size && (BUCKETS.array + i + BUCKETS.horizontal)->size != 0){
                checkBuckets(currBucket, BUCKETS.array + i + BUCKETS.horizontal, deltaTime);
            }

            if(i + BUCKETS.horizontal + 1 < BUCKETS.size && (BUCKETS.array + i + BUCKETS.horizontal + 1)->size != 0){
                checkBuckets(currBucket, BUCKETS.array + i + BUCKETS.horizontal + 1, deltaTime);
            }

            if(i + 1 < BUCKETS.size && i + 1 % BUCKETS.horizontal != 0 && (BUCKETS.array + i + 1)->size != 0){
                checkBuckets(currBucket, BUCKETS.array + i + 1, deltaTime);
            }
        }

        for(int i = 0; i < NUM_INSTANCES; i++){
            float absX = fabsf(*CIRCLES[i].xPos);
            float absY = fabsf(*CIRCLES[i].yPos);

            if((1.0f - absX) * xConversion  <= CIRCLE_RADIUS) {
                CIRCLES[i].vector.xComp *= -1;
            }
            if((1.0f - absY) * yConversion  <= CIRCLE_RADIUS) {
                CIRCLES[i].vector.yComp *= -1;
            }

            moveCircle(&CIRCLES[i], deltaTime / 2);
        }
    }

    for(int i = 0; i < NUM_INSTANCES; i++) {
        VECTOR_COMP[i * 2] = CIRCLES[i].vector.xComp;
        VECTOR_COMP[i * 2 + 1] = CIRCLES[i].vector.yComp;
    }
}

// Resize callback for updating the circle sizes in the gpu
void screenResize() {
    for(int i = 0; i < 12; i += 2){
        CIRCLE_VERTS[i] = points[i] * CIRCLE_RADIUS * 2 / DSP_WIDTH;
        CIRCLE_VERTS[i + 1] = points[i + 1] * CIRCLE_RADIUS * 2 / DSP_HEIGHT;
    }
}

void passiveMouse(int x, int y) {
    if(MOUSE_ACTIVE){
        MOUSE_X = x;
        MOUSE_Y = y;
    }
}

void mouseActivation(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON ){
        MOUSE_ACTIVE = (state == GLUT_DOWN);
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

    BUCKETS.horizontal = ceil(DSP_WIDTH / (AOE * CIRCLE_RADIUS / 1.375f));
    BUCKETS.vertical = ceil(DSP_HEIGHT / (AOE * CIRCLE_RADIUS / 1.375f));

    BUCKETS.size = BUCKETS.horizontal * BUCKETS.vertical;

    BUCKETS.array = calloc(BUCKETS.size, sizeof(Bucket));
}

static void loadBuckets() { //TODO: load vertical x horizontal to make threading easier
    for(int i = 0; i < BUCKETS.size; i++){
        BUCKETS.array[i].size = 0;
    }
    Bucket *inBucket;

    for(int i = 0; i < NUM_INSTANCES; i++){
        int xPos = (int) (((*CIRCLES[i].xPos + 1) / 2) * DSP_WIDTH) / (AOE * CIRCLE_RADIUS / 1.375f);
        int yPos = (int) (((*CIRCLES[i].yPos + 1) / 2) * DSP_HEIGHT) / (AOE * CIRCLE_RADIUS / 1.375f);


        if(xPos + yPos * BUCKETS.horizontal >= BUCKETS.size || xPos + yPos * BUCKETS.horizontal < 0){
            continue;
        }

        inBucket = BUCKETS.array + xPos + yPos * BUCKETS.horizontal;

        if(inBucket->circArray == NULL){
            inBucket->capacity = 16;
            inBucket->size = 0;
            inBucket->circArray = calloc(inBucket->capacity, sizeof(Circle*));
        }
        if(inBucket->size == inBucket->capacity){
            inBucket->circArray = realloc(inBucket->circArray, sizeof(Circle**) * (inBucket->capacity *= 2));
        }
        inBucket->circArray[inBucket->size++] = CIRCLES + i;
    }
}

float distance(float x1, float y1, float x2, float y2) {
    float leftX = (x1 + 1) * (float) DSP_WIDTH / 2; // TODO: DRY
    float leftY = (y1 + 1) * (float) DSP_HEIGHT / 2;
    float rightX = (x2 + 1) * (float) DSP_WIDTH / 2;
    float rightY = (y2 + 1) * (float) DSP_HEIGHT / 2;


    return sqrtf(powf(rightX - leftX, 2) + powf(rightY - leftY, 2));
}


static void applyForce(Vector *dest, Vector src, float deltaTime) {
    dest->xComp += src.xComp * deltaTime;
    dest->yComp += src.yComp * deltaTime;
}

static void moveCircle(Circle *circle, float deltaTime) {
    static float limit = 0.9;

    if(fabsf(circle->vector.xComp) > limit || isnan(circle->vector.xComp)) {
        circle->vector.xComp = (circle->vector.xComp > 0) ? limit : -limit;
    }
    if(fabsf(circle->vector.yComp) > limit || isnan(circle->vector.yComp)) {
        circle->vector.yComp = (circle->vector.yComp > 0) ? limit : -limit;
    }
    *circle->xPos += circle->vector.xComp * deltaTime;
    *circle->yPos += circle->vector.yComp * deltaTime;
}

static float repellingForce(const Circle *left, const Circle *right) {
    float leftX = (*left->xPos + 1) * (float) DSP_WIDTH / 2; // TODO: DRY
    float leftY = (*left->yPos + 1) * (float) DSP_HEIGHT / 2;
    float rightX = (*right->xPos + 1) * (float) DSP_WIDTH / 2;
    float rightY = (*right->yPos + 1) * (float) DSP_HEIGHT / 2;


    float distance = sqrtf(powf(rightX - leftX, 2) + powf(rightY - leftY, 2));  // TODO: DRY

    distance *= 1.375f / CIRCLE_RADIUS;

    if(distance > AOE) {
        return 0;
    }

    // TODO: Different modes?

    //float force = -(740.183506944 * (powf(distance, 6) - 61681.958912)) / powf(distance, 13); // a = 0.01, c = 5.6
    //float force = -(7401.83506944 * (powf(distance, 6) - 61681.958912)) / powf(distance, 13); // a = 0.01, c = 5.6
    //float force = -(74018.3506944 * (powf(distance, 6) - 61681.958912)) / powf(distance, 13); // a = 0.1, c = 5.6
    //float force = -(740183.506944 * (powf(distance, 6) - 61681.958912)) / powf(distance, 13); // a = 1, c = 5.6

    float force = -(426001.76688 * (powf(distance, 6) - 1775.007362)) / powf(distance, 13); // a = 20, c = 2
    //float force = -(740183.506944 * (powf(distance, 6) - 61681.958912)) / powf(distance, 13); // a = 1, c = 5.6
    //float force = -(740183.506944 * (powf(distance, 6) - 61681.958912)) / powf(distance, 13); // a = 1, c = 5.6

    if(force > 100) {
        return 100;
    }
    return force;
}

static void checkBuckets(Bucket *main, Bucket *adj, float deltaTime) {
    if(adj->size == 0){
        return;
    }
    for(int i = 0; i < main->size; i++){
        for(int k = 0; k < adj->size; k++){

            if(fabsf(*main->circArray[i]->xPos - *adj->circArray[k]->xPos) * DSP_WIDTH / 2 * 1.375 / CIRCLE_RADIUS > AOE) continue;

            float force = repellingForce(main->circArray[i], adj->circArray[k]);

            if(force != 0){
                hits++;
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
            } else {
                misses++;
            }
        }
    }
}
