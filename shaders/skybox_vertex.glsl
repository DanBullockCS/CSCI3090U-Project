#version 330 core

uniform mat4 u_MV;

layout(location = 0) in vec3 position;

out vec3 texture_direction;

void main(){
    // Cubemap position
    texture_direction = position;

    // Set the position
    gl_Position = u_MV * vec4(position, 1.0);
}
