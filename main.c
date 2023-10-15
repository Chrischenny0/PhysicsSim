#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


// TODO: Clean everything up
// TODO: Make ball object with vector and position
// TODO: Load screen size as uniform to allow for drawing fragment shader correctly & Use positions to draw ball in fragment shader
// TODO: write sim program


typedef struct vector{
    float xComp;
    float yComp;
} vector;

// Physics structs
typedef struct ball{
    float *position;
    vector vector;
} ball;

vector gravityVec = {0.0f, -0.01f};

// GLOBALS
GLuint baseVBO;
float *sharePositions;

const int radius = 22;

int numOfBalls = 150;

int width = 700;
int height = 500;

float points[12] = {
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f
};

float baseVertices[12];

ball *balls;


const GLchar *vertexShaderSrc = "#version 460\n"
                                "\n"
                                "layout (location = 0) in vec2 pos;\n"
                                "layout (location = 2) in vec2 offset;\n"
                                "\n"
                                "void main() {\n"
                                "    gl_Position = vec4(pos + offset, 0, 1);\n"
                                "}";

const GLchar *fragmentShaderSrc = "#version 460\n"
                                  "out vec4 fragColor;\n"
                                  "void main() {\n"
                                  "    fragColor = vec4(1, 0, 0, 1);"
                                  "}";


GLuint createAndLoadVBO(GLenum usage, void* array, int size){
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, array, usage);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}

GLuint createShader(GLenum shaderType, const GLchar *string){
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

GLuint createProgramAttachShaders(GLuint vertexShader, GLuint fragmentShader){
    GLuint program = glCreateProgram();

    if(program == 0){
        printf("Error creating shader program");
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    return program;
}

void initGL(){

    //Create vbo and fill it with the vertices
    baseVBO = createAndLoadVBO(GL_STATIC_DRAW, baseVertices, sizeof(baseVertices));

    //Create offset VBO
    GLuint instanceVBO = createAndLoadVBO(GL_DYNAMIC_DRAW, NULL, sizeof(float) * numOfBalls * 2);


    //Create vertex shader, specify script, compile, and check for errors
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    // Create program and attach the shaders
    GLuint shaderProgram = createProgramAttachShaders(vertexShader, fragmentShader);

    //Get the input attribute "pos"
    GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");

    // Create a vertex array object (VAO)
    GLuint  vao;
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

    sharePositions = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

    glVertexAttribDivisor(startIndex,1);

    // Set current buffer to null
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Use this created program
    glUseProgram(shaderProgram);
}

void initStaticPhysics(){
    // Set size constraints to make the squares square
    for(int i = 0; i < 12; i += 2){
        baseVertices[i] = points[i] * radius / width;
        baseVertices[i + 1] = points[i + 1] * radius / height;
    }
}

void initDynamicPhysics(){
    balls = calloc(numOfBalls, sizeof(ball));

    float row = 0.7;
    float column = -0.8;
    for(int i = 0; i < 100; i++){
        for(int k = 0; k < 10; k++){
            if(numOfBalls < i * 10 + k){
                goto done;
            }
            sharePositions[(i * 20) + (k * 2)] = column += 0.15f;
            sharePositions[(i * 20) + (k * 2) + 1] = row;
        }
        row -= 0.15;
        column = -0.8;
    }
    done:

    for(int i = 0 ; i < numOfBalls; i++){
        balls[i].position = sharePositions + i * 2;
    }
}


struct timespec start, end;

void applyVector(vector *dest, vector src){
    dest->xComp += src.xComp;
    dest->yComp += src.yComp;
}

void moveParticle(ball *ball, float deltaTime){
    ball->position[0] += ball->vector.xComp * deltaTime;
    ball->position[1] += ball->vector.yComp * deltaTime;
}

void display(){
    // FPS COUNTER
//    static int iterations = 0;
//
//    if(start.tv_nsec == 0){
//        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
//    }
//    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
//
//    if(end.tv_sec - start.tv_sec >= 1){
//        printf("%d\n", iterations);
//        iterations = 0;
//        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
//    }
//
//    iterations++;
//    // FPS COUNTER


    for(int i = 0; i < numOfBalls; i++){
        applyVector(&balls[i].vector, gravityVec);
    }


    for(int i = 0; i < numOfBalls; i++){
        moveParticle(&balls[i], 0.001f);
    }

    glClearColor(0,0,0,0);

    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numOfBalls);

    glutSwapBuffers();
}

void resize(int newWidth, int newHeight){
    height = newHeight;
    width = newWidth;

    for(int i = 0; i < 12; i += 2){
        baseVertices[i] = points[i] * radius / width;
        baseVertices[i + 1] = points[i + 1] * radius / height;
    }

    glBindBuffer(GL_ARRAY_BUFFER, baseVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(baseVertices), baseVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glViewport(0,0,newWidth, newHeight);
}

void idleFunction(){
    glutPostRedisplay();
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

    // Set opengl running functions
    glutDisplayFunc(display);
    glutIdleFunc(idleFunction);
    glutReshapeFunc(resize);
    glutMainLoop();

    // FREEDOM
    free(balls);
    return 0;
}