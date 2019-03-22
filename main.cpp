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

// #define STB_IMAGE_IMPLEMENTATION
// #include "apis/stb_image.h"

#include "objectModel.cpp"
#include "shader.hpp"

using namespace glm;

int width = 640;
int height = 480;


// Model related objects
// The attribute for the vertices
GLint position_attribute;
// The multiple vertex object buffers
vector<GLuint> object_vbos;
// The objects themselves
std::vector<objectModel> objects;

// GLuint textureID;

// Camera related objects
// The position of our eye at default
glm::vec3 eyePosition(0, 0, 180);
// How much to rotate the scene
float rotY = 0.0f;
// How much to scale the Scene
float scaleY = 1.0f;
// Projection matrix - perspective projection
glm::mat4 projectionMatrix;
// View matrix - orient everything around our preferred view
glm::mat4 viewMatrix;

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

static void createObject(string objects_files[], int size) {
    for (int i = 0; i < size; i++) {
        objects.push_back(objectModel(objects_files[i]));

        object_vbos.push_back(0);
        // FIRST OBJECT
        // VERTEX BUFFER OBJECTS (vbo)
        glGenBuffers(1, &object_vbos[i]);  // Create a buffer
        // Send the buffer to the GPU and make it active
        glBindBuffer(GL_ARRAY_BUFFER, object_vbos[i]);
        // Upload the data to the buffer
        // glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data_box),
        // vertex_data_box,
        //              GL_STATIC_DRAW);

        glBufferData(GL_ARRAY_BUFFER, objects[i].vertices.size() * sizeof(vec3),
                     &objects[i].vertices[0], GL_STATIC_DRAW);
    }
}

// static void createTexture(std::string filename) {
//   int imageWidth, imageHeight;
//   int numComponents; // how any values are used to represent each pixel

//    // load the image data into a bitmap
//    // stbi_load from apis/stb_image.h
//    unsigned char *bitmap = stbi_load(filename.c_str(), &imageWidth,
//    &imageHeight, &numComponents, 4);

//    // generate a texture name
//    glGenTextures(1, &textureID);

//    // make the texture active
//    glBindTexture(GL_TEXTURE_2D, textureID);

//    // make a texture mip map
//    glGenerateTextureMipmap(textureID);
//    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

//    // specify the functions to use when shrinking/enlarging the texture image
//    // mipmap
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//    GL_LINEAR_MIPMAP_NEAREST);

//    // specify the tiling parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//    // send the data to OpenGL
//    if (bitmap) {
//      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight,
//                   0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
//    } else {
//      std::cout << "Failed to load texture" << std::endl;
//    }

//    // bind the texture to unit 0
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glActiveTexture(GL_TEXTURE0);

//    // free the bitmap data
//    stbi_image_free(bitmap);
// }

static void drawObject(GLuint programID, objectModel object, GLuint vbo,
                       mat4 MVP, vec4 color) {
    // Use the shaders in the program (only 1 shader can be used at a time)
    glUseProgram(programID);

    // LOAD THE TRANSFORMATIONS
    GLuint mvpMatrixId = glGetUniformLocation(programID, "u_MVP");
    glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &MVP[0][0]);

    // // texture sampler - a reference to the texture we've previously created
    // // send the texture id to the texture sampler
    // GLuint textureUniformID = glGetUniformLocation(programID,
    // "textureSampler"); glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, textureID);
    // glUniform1i(textureUniformID, 0);

    // Apply the color
    GLuint color_attribute = glGetUniformLocation(programID, "u_color");
    glUniform4fv(color_attribute, 1, (GLfloat *)&color[0]);

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
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0,
                          (void *)0);
    glEnableVertexAttribArray(position_attribute);

    glDrawArrays(GL_TRIANGLES, 0, object.vertices.size() * 3);

    // DRAW OBJECTS
    // Disable the vertex attribute array
    glDisableVertexAttribArray(position_attribute);
}

static void render(GLFWwindow *window, GLuint programID) {
    // Turn on depth buffering (dont render objects overtop of eachother)
    glEnable(GL_DEPTH_TEST);

    // CLEAR SCREEN
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate the model matrix (transformations for the model)
    glm::vec3 rotationAxis(0, 1, 0);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix =
        glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(1, 0, 0));
    modelMatrix =
        glm::rotate(modelMatrix, glm::radians(rotY), glm::vec3(0, 1, 0));
    modelMatrix =
        glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleY, scaleY, scaleY));

    // Calculate the Model View Projection (MVP) matrix
    glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

    // Render the objects
    for (int i = 0; i < objects.size(); i++) {
        drawObject(programID, objects[i], object_vbos[i], mvp,
                   vec4(i, 0, 1, 1.0));
    }
}

static GLFWwindow *init_opengl() {
    // The variable to hold the window in
    GLFWwindow *window;

    // Init GLFW
    if (!glfwInit()) exit(EXIT_FAILURE);

// Some definitions for MacOS to load properly
#ifdef __APPLE__
    // We need to explicitly ask for a 3.2 context on MacOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    // Create the window
    window = glfwCreateWindow(width, height, "OpenGL Project", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Make the window active
    glfwMakeContextCurrent(window);

    // Use experimental features of GLEW
    glewExperimental = GL_TRUE;
    // Init GLEW
    glewInit();

    // Throw an error if we don't have opengl 3.2
    if (!GLEW_VERSION_3_2) {
        std::cerr << "OpenGL 3.2 not available" << std::endl;
    }

    // VERTEX ARRAY OBJECTS (VAO)
    // VAO's stores links between attributes and vbo's (the object buffer)
    GLuint vao;
    // Create a vao
    glGenVertexArrays(1, &vao);
    // Bind the vao
    glBindVertexArray(vao);

    return window;
}

int main(void) {
    // Init the scene
    GLFWwindow *window = init_opengl();

    // Catch keyboard keys
    glfwSetKeyCallback(window, key_callback);
    // Catch window resizing
    glfwSetFramebufferSizeCallback(window, resize_window);

    // The objects that we wish to load in
    string object_files[] = {"newHead.obj", "my_sphere.obj"};
    // Create/Load the objects
    createObject(object_files, sizeof(object_files) / sizeof(object_files[0]));

    // Load and prepare the texture
    // createTexture("textures/grass.png");

    // Load the shaders
    GLuint programID =
        createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Calculate the perspective in the scene
    calculate_perspective((float)width / height);

    // Calculate the view matrix (where we're looking at)
    viewMatrix =
        glm::lookAt(eyePosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window)) {
        // Render the scene
        render(window, programID);

        // Display the image
        glfwSwapBuffers(window);
        glfwPollEvents();
        // glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
