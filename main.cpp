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

#include "objectModel.cpp"
#include "shader.hpp"

using namespace glm;

// Window is resizeable
int width = 640;
int height = 480;

// Model related objects
// The multiple vertex object buffers
std::vector<GLuint> object_vbos;
// The objects themselves
std::vector<objectModel> objects;
// The multiple texture coordinates
std::vector<GLuint> object_textcoord_vbos;
// The multiple normals
std::vector<GLuint> object_normal_vbos;
// The IDs for the texture
std::vector<GLuint> texture_locs;
// initial offset of each object
std::vector<glm::vec3> initial_offset;

// Camera related objects
// The position of our eye at default
glm::vec3 eyePosition(0, 90, 360);
// How much to scale the Scene
float scaleY = 1.0f;
// Projection matrix - perspective projection
glm::mat4 projection_matrix;
// View matrix - orient everything around our preferred view
glm::mat4 view_matrix;

// The velocity to move the object by
float veloc = 2.0f;
// The amount to rotate the ball by
float rot = 0.5f;
// How much to rotate the sphere
float rot_x = 0.0f;
float rot_y = 0.0f;

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
    // Get a keypress and move the objects
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else {
        if (key == GLFW_KEY_W) {
            initial_offset[0].z -= veloc;
            rot_x -= rot;
        } else if (key == GLFW_KEY_S) {
            initial_offset[0].z += veloc;
            rot_x += rot;
        }
        if (key == GLFW_KEY_A) {
            initial_offset[0].x -= veloc;
            rot_y -= rot;
        } else if (key == GLFW_KEY_D) {
            initial_offset[0].x += veloc;
            rot_y += rot;
        }
    }
}

static void calculate_perspective(float aspect_ratio) {
    // Calculate the projection matrix with a 45 degree field of view
    projection_matrix =
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

static void create_object(std::string objects_files[], int size) {
    for (int i = 0; i < size; i++) {
        objects.push_back(objectModel(objects_files[i]));
        object_vbos.push_back(0);
        object_textcoord_vbos.push_back(0);
        object_normal_vbos.push_back(0);

        // FIRST OBJECT
        // VERTEX BUFFER OBJECTS (vbo)
        glGenBuffers(1, &object_vbos[i]);  // Create a buffer
        // Send the buffer to the GPU and make it active
        glBindBuffer(GL_ARRAY_BUFFER, object_vbos[i]);
        // Upload the data to the buffer
        glBufferData(GL_ARRAY_BUFFER, objects[i].vertices.size() * sizeof(vec3),
                     &objects[i].vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &object_textcoord_vbos[i]);  // Create a buffer
        // Send the buffer to the GPU and make it active
        glBindBuffer(GL_ARRAY_BUFFER, object_textcoord_vbos[i]);
        // Upload the data to the buffer
        glBufferData(GL_ARRAY_BUFFER, objects[i].uvs.size() * sizeof(vec2),
                     &objects[i].uvs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &object_normal_vbos[i]);  // Create a buffer
        // Send the buffer to the GPU and make it active
        glBindBuffer(GL_ARRAY_BUFFER, object_normal_vbos[i]);
        // Upload the data to the buffer
        glBufferData(GL_ARRAY_BUFFER, objects[i].normals.size() * sizeof(vec3),
                     &objects[i].normals[0], GL_STATIC_DRAW);
    }
}

static void create_texture(std::string textures[], int size) {
    // size hard coded to 5 since c++ compiler was not to fond of passing a size
    GLuint texture_ids[5];
    // generate a texture name
    glGenTextures(size, texture_ids);

    for (int i = 0; i < size; i++) {
        std::string filename = textures[i];

        // make the texture active
        glBindTexture(GL_TEXTURE_2D, texture_ids[i]);

        int imageWidth, imageHeight;
        int numComponents;  // how any values are used to represent each pixel

        // load the image data into a bitmap
        // stbi_load from apis/stb_image.h
        stbi_set_flip_vertically_on_load(true);
        unsigned char *bitmap = stbi_load(filename.c_str(), &imageWidth,
                                          &imageHeight, &numComponents, 4);

        // make a texture mip map
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

        // specify the functions to use when shrinking/enlarging the texture
        // image mipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_NEAREST);

        // specify the tiling parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // send the data to OpenGL
        if (bitmap) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture" << std::endl;
        }

        // free the bitmap data
        stbi_image_free(bitmap);

        texture_locs.push_back(texture_ids[i]);
    }
}

static void draw_object(GLuint programID, objectModel object, GLuint vertex_vbo,
                        GLuint uv_vbo, GLuint normal_vbo, GLuint textureID, mat4 MVP, mat4 MV,
                        vec4 color, bool texture_flag) {
    // Use the shaders in the program (only 1 shader can be used at a time)
    glUseProgram(programID);

    // Get the attributes from the shader
    GLint vertex_attribute = glGetAttribLocation(programID, "position");
    GLuint mvp_attribute = glGetUniformLocation(programID, "u_MVP");
    GLuint mv_attribute = glGetUniformLocation(programID, "u_MV");
    GLuint color_attribute = glGetUniformLocation(programID, "u_color");
    GLuint light_pos_attribute = glGetUniformLocation(programID, "u_light_pos");
    GLuint eye_pos_attribute = glGetUniformLocation(programID, "u_eye_pos");
    GLuint shine_attribute = glGetUniformLocation(programID, "u_shininess");
    GLuint texture_uniform_attribute =
        glGetUniformLocation(programID, "u_texture_sampler");
    GLuint texture_flag_attribute =
        glGetUniformLocation(programID, "u_lighting_switch");
    GLint texture_coords_attribute =
        glGetAttribLocation(programID, "texture_coords");
    GLint texture_normal_attribute =
        glGetAttribLocation(programID, "texture_normal");

    // Send the transformations
    glUniformMatrix4fv(mvp_attribute, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(mv_attribute, 1, GL_FALSE, &MV[0][0]);
    // Send the color information
    glUniform4fv(color_attribute, 1, (GLfloat *)&color[0]);
    // Send the light position information
    glUniform3f(light_pos_attribute, 0, 10, 0);
    // Send the eye position information
    glUniform3f(eye_pos_attribute, eyePosition.x, eyePosition.y, eyePosition.z);
    // Send the shininess information
    glUniform1f(shine_attribute, 2.5f);
    // Send the texture coords
    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
    glEnableVertexAttribArray(texture_coords_attribute);
    glVertexAttribPointer(texture_coords_attribute, 2, GL_FLOAT, GL_FALSE, 0,
                          nullptr);
    // Send the normals
    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
    glEnableVertexAttribArray(texture_normal_attribute);
    glVertexAttribPointer(texture_normal_attribute, 3, GL_FLOAT, GL_FALSE, 0,
                          nullptr);
    // Send the texture data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // glUniform1i(texture_uniform_attribute, 0);
    glUniform1f(texture_flag_attribute, texture_flag);
    // Send the vertex_data
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glEnableVertexAttribArray(vertex_attribute);
    glVertexAttribPointer(vertex_attribute, 3, GL_FLOAT, GL_FALSE, 0,
                          (void *)0);
    // Draw the elements in the buffer
    glDrawArrays(GL_TRIANGLES, 0, object.vertices.size() * 3);
    // Disable the attribute arrays
    glDisableVertexAttribArray(vertex_attribute);
    glDisableVertexAttribArray(texture_normal_attribute);
    glDisableVertexAttribArray(texture_coords_attribute);
}

static void render(GLFWwindow *window, GLuint programID) {
    // Turn on depth buffering (dont render objects overtop of eachother)
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // CLEAR SCREEN
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the objects
    for (int i = 0; i < objects.size(); i++) {
        // Used to turn lighting off for specific objects
        bool use_lighting = false;

        // Set the axis for rotation
        glm::vec3 rotationAxis(0, 0, 0);
        // Set the identity matrix
        glm::mat4 model_matrix = glm::mat4(1.0f);

        if (rot_x > 360) {
            rot_x = 0;
        } else if (rot_x < 0) {
            rot_x = 360;
        }
        if (rot_y > 360) {
            rot_y = 0;
        } else if (rot_y < 0) {
            rot_y = 360;
        }

        // Sphere translation/rotation
        if (i == 0) {
            // Move the object over
            model_matrix =
                glm::translate(model_matrix, initial_offset[i]);
            // Rotate it depending on the value
            model_matrix = glm::rotate(model_matrix, rot_x, glm::vec3(1, 0, 0));
            model_matrix = glm::rotate(model_matrix, rot_y, glm::vec3(0, 0, 1));

            use_lighting = true;
        // Plane (ground)
        } else if (i == 1) {
            use_lighting = false;
            model_matrix =
                glm::translate(model_matrix, initial_offset[i]);
        // Cube
        } else if (i == 2) {
            use_lighting = false;
            model_matrix =
                glm::translate(model_matrix, initial_offset[i]);
        // Goal post
        } else if (i == 3) {
            use_lighting = false;
            model_matrix =
                glm::translate(model_matrix, initial_offset[i]);
            model_matrix = glm::rotate(model_matrix, -40.0f, glm::vec3(0, 1, 0));
        // Bench
        } else if (i == 4) {
            use_lighting = true;
            model_matrix = glm::translate(model_matrix, initial_offset[i]);
             model_matrix = glm::rotate(model_matrix, -40.0f, glm::vec3(0, 1, 0));
            // Scale the large static bench down
            model_matrix =  glm::scale(model_matrix, glm::vec3(0.005, 0.005, 0.005));
        }

        // Calculate the scale of the object
        // model_matrix =
        //     glm::scale(model_matrix, glm::vec3(scaleY, scaleY, scaleY));

        // Draw the object
        draw_object(programID, objects[i], object_vbos[i],
                    object_textcoord_vbos[i], object_normal_vbos[i], texture_locs[i],
                    projection_matrix * view_matrix * model_matrix,
                    view_matrix * model_matrix, vec4(i / 10, 0, 1, 1.0),
                    use_lighting);
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
    // Enable antialising
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Create the window
    window = glfwCreateWindow(width, height, "CSCI3090U OpenGL Project", NULL, NULL);
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
    // The initial positions of the obj objects
    initial_offset.push_back(glm::vec3(-75, 20, 150));         // sphere
    initial_offset.push_back(glm::vec3(0, 0, 0));              // plane (ground)
    initial_offset.push_back(glm::vec3(0, 0, 0));              // cube
    initial_offset.push_back(glm::vec3(0, 0, 0));              // soccer net
    initial_offset.push_back(glm::vec3(25, 20, 250));          // bench
    // The first object must be the ball, the last object must be the cube
    string object_files[] = {"meshes/my_sphere.obj",
                             "meshes/plane.obj",
                             "meshes/cube.obj" ,
                             "meshes/gate.obj",
                             "meshes/Bench3.obj"};
    // Create/Load the objects
    create_object(object_files, sizeof(object_files) / sizeof(object_files[0]));

    // Load and prepare the texture
    std::string textures[] = {"textures/soccer.png",
                              "textures/grass.jpg",
                              "textures/cartoon_sky.png",
                              "textures/goal.png",
                              "textures/planks.jpg"};
    create_texture(textures, sizeof(textures) / sizeof(textures[0]));

    // Load the shaders
    GLuint programID =
        createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Calculate the perspective in the scene
    calculate_perspective((float)width / height);

    // Calculate the view matrix (where we're looking at)
    view_matrix =
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
