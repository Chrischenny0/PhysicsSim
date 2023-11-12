#include "graphics.h"


void (*PHYSICS_LOOP)(float) = NULL;

// Physics callback for resizing
void (*PHYSICS_RESIZE)() = NULL;


// Graphics globals
int CIRCLE_RADIUS = 0;      // Radius of each circle
int NUM_INSTANCES = 0;      // Num of circles to draw
float CIRCLE_VERTS[12];     // Vertices of the base sprite
float *INSTANCE_POS = NULL; // Position of each instance (created and share by GPU)
float *VECTOR_COMP = NULL;


// Display height and width
int DSP_WIDTH = 0;
int DSP_HEIGHT = 0;


// Local globals
static GLuint baseVBO;       // VBO for initial sprite
static int prevTime;         // For delta time calculation
static GLuint shaderProgram; // Maintain the shader program for updating
static struct timespec prev; // Used for calculating the fps


// SHADERS
static GLchar vertexShaderSrc[1024];

static GLchar fragmentShaderSrc[1024];


static GLuint createAndLoadVBO(GLenum usage, void* array, int size){
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, array, usage);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}

static GLuint createShader(GLenum shaderType, const GLchar *string) {
    GLuint shader = glCreateShader(shaderType);

    if (shader == 0) { // Error: Cannot create shader object
        printf("Error creating shaders");
        return 0;
    }

    // Attach source code to this object
    glShaderSource(shader, 1, &string, NULL);
    glCompileShader(shader);

    GLint compileStatus;

    // check for compilation status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if(!compileStatus){
        char cMessage[2048];
        glGetShaderInfoLog(shader, 2048, &compileStatus, cMessage);

        printf("Cannot Compile Shader: %s", cMessage);
        glDeleteShader(shader);
    }

    return shader;
}

static GLuint createProgramAttachShaders(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();

    if(program == 0){
        printf("Error creating shader program");
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    return program;
}

void initGL() {

    //TODO: Make this not hardcoded
    FILE* currFile = fopen("circleFragment.frag", "r");
    fread(fragmentShaderSrc, sizeof(char), 1024, currFile);
    fclose(currFile);

    currFile = fopen("circleVertex.vert", "r");
    fread(vertexShaderSrc, sizeof(char), 1024, currFile);
    fclose(currFile);

    //Create vbo and fill it with the vertices
    baseVBO = createAndLoadVBO(GL_STATIC_DRAW, CIRCLE_VERTS, sizeof(CIRCLE_VERTS));

    //Create offset VBO
    GLuint instanceVBO = createAndLoadVBO(GL_DYNAMIC_DRAW, NULL, sizeof(float) * NUM_INSTANCES * 2);

    //Create vector component VBO
    GLuint vectorVBO = createAndLoadVBO(GL_DYNAMIC_DRAW, NULL, sizeof(float) * NUM_INSTANCES * 2);


    //Create vertex shader, specify script, compile, and check for errors
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    // Create program and attach the shaders
    shaderProgram = createProgramAttachShaders(vertexShader, fragmentShader);

    //Get the input attribute "pos"
    GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");

    // Create a vertex array object (VAO)
    GLuint vao;
    glGenVertexArrays(1, &vao);

    // Bing the VAO so all data is added
    glBindVertexArray(vao);

    // Bind the vertex VBO, set input attributes and enable the attributes
    glBindBuffer(GL_ARRAY_BUFFER, baseVBO);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0,0);
    glEnableVertexAttribArray(posAttrib);

    int startIndex = 2;
    // Bind the instance VBO, assign attributes, enable attributes, and set divisor
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(startIndex,2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
    glEnableVertexAttribArray(startIndex);

    INSTANCE_POS = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

    glVertexAttribDivisor(startIndex++,1);

    // Set current buffer to null
    glBindBuffer(GL_ARRAY_BUFFER, vectorVBO);
    glVertexAttribPointer(startIndex,2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
    glEnableVertexAttribArray(startIndex);

    VECTOR_COMP = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

    glVertexAttribDivisor(startIndex,1);

    // Use this created program
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(shaderProgram);


    // Set the uniforms inside the shader
    GLint uniform = glGetUniformLocation(shaderProgram, "size");

    float tmp[] = {DSP_WIDTH, DSP_HEIGHT};
    glUniform2fv(uniform, 1, tmp);

    uniform = glGetUniformLocation(shaderProgram, "radius");

    glUniform1f(uniform, CIRCLE_RADIUS - 1);


    // Initialize fps counter
    clock_gettime(CLOCK_MONOTONIC_RAW, &prev);

    prevTime = glutGet(GLUT_ELAPSED_TIME);
}

void display() {
    // FPS COUNTER
    static int iterations = 0;

    struct timespec start;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    if(start.tv_sec - prev.tv_sec >= 1){
        printf("%d\n", iterations);
        iterations = 0;
        clock_gettime(CLOCK_MONOTONIC_RAW, &prev);
    }


    iterations++;
    // FPS COUNTER

    // Delta Time maker
    int currTime = glutGet(GLUT_ELAPSED_TIME);

    float deltaTime = (currTime - prevTime) / 1000.0;

    prevTime = currTime;

    PHYSICS_LOOP(deltaTime);


    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, NUM_INSTANCES);

    glutSwapBuffers();
}

void resize(int newWidth, int newHeight) {
    DSP_HEIGHT = newHeight;
    DSP_WIDTH = newWidth;

    PHYSICS_RESIZE();

    glBindBuffer(GL_ARRAY_BUFFER, baseVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CIRCLE_VERTS), CIRCLE_VERTS);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    GLint sizeUniform = glGetUniformLocation(shaderProgram, "size");

    float tmp[] = {DSP_WIDTH, DSP_HEIGHT};
    glUniform2fv(sizeUniform, 1, tmp);

    glViewport(0,0,newWidth, newHeight);
}

void idle() {
    glutPostRedisplay();
}
