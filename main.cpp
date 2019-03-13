#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "shader.hpp"

using namespace glm;

GLint position_attribute;
GLuint vbo;
GLuint ibo;

GLuint programID;

GLfloat vertexData[] = {0, 0, 0, 
                        -1.0f, -1.0f, 0, 
                        1.0f, -1.0f, 0};

GLuint indexData[] = {0, 1, 2};


// NOT WORKING ATM
static const char *loadFile(std::string filename) {
    std::string shader = "";

    std::ifstream read;
    read.open(filename);

    std::string line;
    while (!read.eof()) {
        std::getline(read, line);
        shader += line + "\n";
    }

    return shader.c_str();
}

// NOT WORKING ATM
static GLuint loadShaders() {
    const char *vertex_string =
        loadFile("shaders/vertex.glsl");  // Get the vertex shader
    const char *fragment_string =
        loadFile("shaders/fragment.glsl");  // Get the fragment shader

    // VERTEX SHADER
    // Create the vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    // Upload vertex shader info into the GPU
    glShaderSource(vertex_shader, 1, &vertex_string, nullptr);
    // Compile the vertex shader
    glCompileShader(vertex_shader);

    // FRAGMENT SHADER
    // Create the fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // Upload fragment shader info into the GPU
    glShaderSource(fragment_shader, 1, &fragment_string, nullptr);
    // Compile the fragment shader
    glCompileShader(fragment_shader);






    // BIND SHADERS TOGETHER
    // Connect the shaders together by creating a program
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    // // Specify which output is for which buffer (for fragment only)
    // glBindFragDataLocation(fragment_shader, 0, "outColor");

    // LINK AND USE SHADERS
    // Link the shader to the program
    glLinkProgram(shader_program);
    glValidateProgram(shader_program);

    glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

    // // Get the variable of "position" from the shader
    // position_attribute = glGetAttribLocation(shader_program, "position");

    return shader_program;
}

static void createObject() {
    // VERTEX ARRAY OBJECTS (VAO)
    // VAO's stores links between attributes and VBO's (the object buffer)
    GLuint vao;
    glGenVertexArrays(1, &vao);  // Create a vao
    // Bind the vao
    glBindVertexArray(vao);

    // VERTEX BUFFER OBJECTS (VBO)
    glGenBuffers(1, &vbo);  // Create a buffer
    // Send the buffer to the GPU and make it active
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Upload the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData,
                 GL_STATIC_DRAW);

    // (IBO)
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint), indexData,
                 GL_STATIC_DRAW);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void render(GLFWwindow *window) {
    // Use the shaders in the program (only 1 shader can be used at a time)
    glUseProgram(programID);

    // Enable the vertex attribute array
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Specify how the input array (the verticies) is inputted into the shader
    // The first val is the input
    // The second val is the number of values in a input
    // The third val specifies the type of the input
    // The fourth val specifies whether the values should be normalized to -1 ->
    // 1 The fifth val is the stride (how many bytes are between each position
    // in the array) The sixth val us the offset (how many bytes from the start
    // of the array the input occurs)
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(position_attribute);

    // CLEAR SCREEN
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // DRAW OBJECTS
    // First val specifies the type of primitive
    // Second val specifies how many verticies to skip at the beginning
    // Third val specifies how many verticies to process
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Set index data and render
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void *)0);

    // Disable the vertex attribute array
    glDisableVertexAttribArray(position_attribute);
}

static GLFWwindow *init_opengl() {
    GLFWwindow *window;

    if (!glfwInit()) exit(EXIT_FAILURE);

#ifdef __APPLE__
    // We need to explicitly ask for a 3.2 context on MacOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    window = glfwCreateWindow(640, 480, "OpenGL Project", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    if (!GLEW_VERSION_3_2) {
        std::cerr << "OpenGL 3.2 not available" << std::endl;
    }

    return window;
}

int main(void) {
    GLFWwindow *window = init_opengl();

    glfwSetKeyCallback(window, key_callback);

    createObject();
    programID = loadShaders();
    // programID = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    while (!glfwWindowShouldClose(window)) {
        render(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
        // glfwWaitEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}