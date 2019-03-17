#version 330 core

in vec3 position;
uniform mat4 u_MVP;
uniform vec4 u_color;

out vec4 v_color;

void main(){
    gl_Position = u_MVP * vec4(position, 1.0);
    v_color = u_color;
}