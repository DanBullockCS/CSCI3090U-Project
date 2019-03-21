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

#define STB_IMAGE_IMPLEMENTATION
#include "apis/stb_image.h"

#include "shader.hpp"

using namespace glm;

int width = 640;
int height = 480;

GLuint programID;
GLint position_attribute;
GLuint vbo_box;
GLuint ibo_box;
GLuint vbo_square;
GLuint ibo_square;
GLuint textureID;

// Projection matrix - perspective projection
glm::mat4 projectionMatrix;
// View matrix - orient everything around our preferred view
glm::mat4 viewMatrix;

// Camera related objects
glm::vec3 eyePosition(0, 0, 180);
float rotY = 0.0f;
float scaleY = 1.0f;

// Define positions
GLfloat vertex_data_box[] = {50,  50,  50,  50, 50,  -50, -50, 50,  50,
                             -50, 50,  -50, 50, -50, 50,  -50, -50, -50,
                             -50, -50, 50,  50, -50, -50, 0,   0,   0,
                             1,   1,   0,   0,  1,   0};

GLfloat vertex_data_square[] = {
    -5.080000, -5.080000, 0.000000,  5.080000, -5.080000, 0.000000,
    -5.080000, -5.080000, 10.160000, 5.080000, -5.080000, 10.160000,
    -5.080000, 5.080000,  10.160000, 5.080000, 5.080000,  10.160000,
    -5.080000, 5.080000,  0.000000,  5.080000, 5.080000,  0.000000};

GLuint index_data_box[] = {6, 0, 2, 6, 4, 0, 2, 3, 5,
                           2, 5, 6, 0, 4, 1, 1, 4, 7};

GLuint index_data_square[] = {2,3,5,4};

// Define colors
vec4 pink(1.0, 0.5, 0.5, 1.0);
vec4 blue(0.0, 0.0, 0.8, 1.0);

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
        if (key == GLFW_KEY_W && action == GLFW_PRESS) {
            move_view(0, 1);
        } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            move_view(0, -1);
        }
        if (key == GLFW_KEY_A && action == GLFW_PRESS) {
            move_view(5, 0);
        } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
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
    // VAO's stores links between attributes and vbo_box's (the object buffer)
    GLuint vao;
    glGenVertexArrays(1, &vao);  // Create a vao
    // Bind the vao
    glBindVertexArray(vao);

    // FIRST OBJECT
    // VERTEX BUFFER OBJECTS (vbo_box)
    glGenBuffers(1, &vbo_box);  // Create a buffer
    // Send the buffer to the GPU and make it active
    glBindBuffer(GL_ARRAY_BUFFER, vbo_box);
    // Upload the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data_box), vertex_data_box,
                 GL_STATIC_DRAW);

    // INDEXED vbo_box (ibo_box)
    glGenBuffers(1, &ibo_box);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_box);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data_box),
                 &index_data_box[0], GL_STATIC_DRAW);

    // SECOND OBJECT
    // VERTEX BUFFER OBJECTS (vbo_box)
    glGenBuffers(1, &vbo_square);  // Create a buffer
    // Send the buffer to the GPU and make it active
    glBindBuffer(GL_ARRAY_BUFFER, vbo_square);
    // Upload the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data_square),
                 vertex_data_square, GL_STATIC_DRAW);

    // INDEXED vbo_box (ibo_box)
    glGenBuffers(1, &ibo_square);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_square);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data_square),
                 &index_data_square[0], GL_STATIC_DRAW);
}

static void createTexture(std::string filename) {
  int imageWidth, imageHeight;
  int numComponents; // how any values are used to represent each pixel

   // load the image data into a bitmap
   // stbi_load from apis/stb_image.h
   unsigned char *bitmap = stbi_load(filename.c_str(), &imageWidth, &imageHeight, &numComponents, 4);

   // generate a texture name
   glGenTextures(1, &textureID);

   // make the texture active
   glBindTexture(GL_TEXTURE_2D, textureID);

   // make a texture mip map
   glGenerateTextureMipmap(textureID);
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

   // specify the functions to use when shrinking/enlarging the texture image
   // mipmap
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

   // specify the tiling parameters
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   // send the data to OpenGL
   if (bitmap) {
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
   } else {
     std::cout << "Failed to load texture" << std::endl;
   }

   // bind the texture to unit 0
   glBindTexture(GL_TEXTURE_2D, textureID);
   glActiveTexture(GL_TEXTURE0);

   // free the bitmap data
   stbi_image_free(bitmap);
}

static void render(GLFWwindow *window, mat4 MVP) {
    // Use the shaders in the program (only 1 shader can be used at a time)
    glUseProgram(programID);

    // Turn on depth buffering (dont render objects overtop of eachother)
    glEnable(GL_DEPTH_TEST);

    // LOAD THE TRANSFORMATIONS
    GLuint mvpMatrixId = glGetUniformLocation(programID, "u_MVP");
    glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &MVP[0][0]);

    // texture sampler - a reference to the texture we've previously created
    // send the texture id to the texture sampler
    GLuint textureUniformID = glGetUniformLocation(programID, "textureSampler");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureUniformID, 0);

    // Apply the color
    GLuint color_attribute = glGetUniformLocation(programID, "u_color");
    glUniform4fv(color_attribute, 1, (GLfloat *)&pink[0]);

    // CLEAR SCREEN
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // FIRST OBJECT
    // Enable the vertex attribute array
    glBindBuffer(GL_ARRAY_BUFFER, vbo_box);
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

    // DRAW OBJECTS
    // Set index data and draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_box);
    glDrawElements(GL_TRIANGLES,
                   (sizeof(index_data_box) / sizeof(index_data_box[0])),
                   GL_UNSIGNED_INT, (void *)0);

    // Disable the vertex attribute array
    glDisableVertexAttribArray(position_attribute);

    // Apply the color
    color_attribute = glGetUniformLocation(programID, "u_color");
    glUniform4fv(color_attribute, 1, (GLfloat *)&blue[0]);

    // SECOND OBJECT
    // Enable the vertex attribute array
    glBindBuffer(GL_ARRAY_BUFFER, vbo_square);
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

    // DRAW OBJECTS
    // Set index data and draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_square);
    glDrawElements(GL_TRIANGLES,
                   (sizeof(index_data_square) / sizeof(index_data_square[0])),
                   GL_UNSIGNED_INT, (void *)0);

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

    // Load and prepare the texture
    createTexture("textures/grass.png");

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
