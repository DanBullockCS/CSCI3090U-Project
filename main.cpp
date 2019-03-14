#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "shader.hpp"

using namespace glm;

int width = 640;
int height = 480;

GLuint programID;
GLint position_attribute;
GLuint vbo;
GLuint ibo;

// projection matrix - perspective projection
glm::mat4 projectionMatrix;
// view matrix - orient everything around our preferred view
glm::mat4 viewMatrix;

// Camera related objects
glm::vec3 eyePosition(90, 0, 0);
float rotY = 0.0f;
float scaleY = 1.0f;

GLfloat vertexData[] = {-1, 1, 0, -1, 0, 0, 1, 0, 0, 1, 1, 0};
std::vector<GLuint> indexData;

static void move_view(int x, int y) {
    // Increment / decrement the view
    if (scaleY + y >= 0 && scaleY + y <= 255) {
        scaleY += y;
    }
    rotY += x;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
    // Get a keypress and move the objects
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else {
        if (key == GLFW_KEY_W) {
            move_view(0, 1);
        } else if (key == GLFW_KEY_S) {
            move_view(0, -1);
        }
        if (key == GLFW_KEY_A) {
            move_view(5, 0);
        } else if (key == GLFW_KEY_D) {
            move_view(-5, 0);
        }
    }
}

static void calculate_perspective(float aspect_ratio) {
    // Calculate the projection matrix with a 45 degree field of view
    projectionMatrix =
        glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 1000.0f);
}

static void resize_window(GLFWwindow *window, GLint w, GLint h) {
    // Take the resolution of thew new window
    width = w;
    height = h;

    // Set the viewport to that size
    glViewport(0, 0, width, height);

    // Get the new aspect ratio of the resolution
    float aspect_ratio = (float)w / (float)h;

    // Calculate the new aspect ratio
    calculate_perspective(aspect_ratio);
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

    indexData.push_back(1);
    indexData.push_back(3);
    indexData.push_back(0);

    indexData.push_back(2);
    indexData.push_back(3);
    indexData.push_back(1);

    // INDEXED VBO (IBO)
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint),
                 &indexData[0], GL_STATIC_DRAW);
}

static void render(GLFWwindow *window, mat4 MVP) {
    // Use the shaders in the program (only 1 shader can be used at a time)
    glUseProgram(programID);

    // LOAD THE TRANSFORMATIONS
    GLuint mvpMatrixId = glGetUniformLocation(programID, "u_MVP");
    glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &MVP[0][0]);

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
    // Set index data and draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, indexData.size(), GL_UNSIGNED_INT, (void *)0);

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

    window = glfwCreateWindow(width, height, "OpenGL Project", NULL, NULL);
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
    GLFWwindow *window = init_opengl();  // Init the scene

    glfwSetKeyCallback(window, key_callback);  // Catch keyboard keys
    glfwSetFramebufferSizeCallback(window,
                                   resize_window);  // Catch window resizing
    createObject();  // Create the objects in the scene

    // Load the shaders
    programID =
        createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Calculate the perspective in the scene
    calculate_perspective((float)width / height);

    // Calculate the view matrix (where we're looking at)
    viewMatrix =
        glm::lookAt(eyePosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window)) {
        // Calculate the model matrix (transformations for the model)
        glm::vec3 rotationAxis(0, 1, 0);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(1, 0, 0));
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(rotY), glm::vec3(0, 1, 0));
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(0, 0, 1));
        modelMatrix =
            glm::scale(modelMatrix, glm::vec3(scaleY, scaleY, scaleY));

        // Calculate the Model View Projection (MVP) matrix
        glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

        // Render the object
        render(window, mvp);

        // Display the image
        glfwSwapBuffers(window);
        glfwPollEvents();
        // glfwWaitEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}