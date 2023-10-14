#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <tgmath.h>


// TODO: Clean everything up
// TODO: Have the vertices stay square through updating based off screen dimension
// TODO: Make ball object with vector and position
// TODO: Use positions to draw ball in fragment shader
// TODO: write sim program

float *testPtr;

float vertices[] = {
        -0.01f,  -0.01f,
        0.01f, 0.01f,
        0.01f, -0.01f,
        -0.01f,  -0.01f,
        0.01f, 0.01f,
        -0.01f, 0.01f
};

float translations[] = {
        -0.5, -0.5, 0.5, 0.5
};

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


GLuint createAndLoadVBO(float array[], int size){
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, array, GL_STATIC_DRAW);

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

void init(){

    //Create vbo and fill it with the vertices
    GLuint vbo = createAndLoadVBO(vertices, sizeof(vertices));

    //Create offset VBO
    GLuint instanceVBO = createAndLoadVBO(translations, sizeof(translations));

    testPtr = glMapNamedBuffer(instanceVBO, GL_READ_WRITE);


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
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0,0);
    glEnableVertexAttribArray(posAttrib);

    int startIndex = 2;
    // Bind the instance VBO, assign attributes, enable attributes, and set divisor
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(startIndex,2, GL_FLOAT, GL_FALSE, 2 * sizeof(float ), NULL);
    glEnableVertexAttribArray(startIndex);

    glVertexAttribDivisor(startIndex,1);

    // Set current buffer to null
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Use this created program
    glUseProgram(shaderProgram);
}


void display(){
    glClearColor(0,0,0,0);

    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 2);

    glutSwapBuffers();
}

void idleFunction(){
    glutPostRedisplay();
}


int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Triangle Using OpenGL");

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glewInit();
    init();

    glutDisplayFunc(display);
    glutIdleFunc(idleFunction);
    glutMainLoop();

    return 0;
}