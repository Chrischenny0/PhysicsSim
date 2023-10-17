#ifndef PHYSICSSIM_GRAPHICS_H
#define PHYSICSSIM_GRAPHICS_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

const int radius = 10;

int numOfBalls = 200;

typedef struct vector{
    float xComp;
    float yComp;
} vector;

GLuint baseVBO;
float *sharePositions;
static int prevTime;

struct timespec start, end;


const GLchar *vertexShaderSrc = "#version 460\n"
                                "layout (location = 0) in vec2 pos;\n"
                                "layout (location = 2) in vec2 offset;\n"
                                "out vec2 coords;\n"
                                "uniform vec2 size;\n"
                                "void main() {\n"
                                "    gl_Position = vec4(pos + offset, 0, 1);\n"
                                "    coords = (offset + 1) * size * 0.5;\n"
                                "}";

const GLchar *fragmentShaderSrc = "#version 460\n"
                                  "out vec4 fragColor;\n"
                                  "in vec2 coords;\n"
                                  "uniform float radius;\n"
                                  "void main() {\n"
                                  "    float dist = distance(gl_FragCoord.xy, coords);\n"
                                  "    if(dist >= radius){\n"
                                  "        discard;\n"
                                  "    }\n"
                                  "    fragColor = vec4(1, 0, 0, 1);\n"
                                  "}";

int width = 400;
int height = 400;

float points[12] = {
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f,  -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f
};

float baseVertices[12];

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

    GLint sizeUniform = glGetUniformLocation(shaderProgram, "size");

    float tmp[] = {width, height};
    glUniform2fv(sizeUniform, 1, tmp);

    sizeUniform = glGetUniformLocation(shaderProgram, "radius");

    glUniform1f(sizeUniform, radius - 1);
}

void (*physicsFunc)(float);

void display(){
    // FPS COUNTER
    static int iterations = 0;

    if(start.tv_nsec == 0){
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    if(end.tv_sec - start.tv_sec >= 1){
        printf("%d\n", iterations);
        iterations = 0;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    }

    iterations++;
    // FPS COUNTER

    int currTime = glutGet(GLUT_ELAPSED_TIME);

    float deltaTime = (currTime - prevTime) / 1000.0;

    prevTime = currTime;

    physicsFunc(deltaTime);


    glClearColor(0,0,0,0);

    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numOfBalls);

    glutSwapBuffers();
    glutPostRedisplay();
}

void resize(int newWidth, int newHeight){
    height = newHeight;
    width = newWidth;

    for(int i = 0; i < 12; i += 2){
        baseVertices[i] = points[i] * radius * 2 / width;
        baseVertices[i + 1] = points[i + 1] * radius * 2 / height;
    }

    glBindBuffer(GL_ARRAY_BUFFER, baseVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(baseVertices), baseVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glViewport(0,0,newWidth, newHeight);
}

#endif //PHYSICSSIM_GRAPHICS_H
