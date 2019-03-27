#version 330 core

// Texture Information
uniform samplerCube cubemap;
uniform vec4 u_color;

in vec3 texture_direction;

out vec4 outColor;

void main(){
    outColor = texture(cubemap, texture_direction);
}
