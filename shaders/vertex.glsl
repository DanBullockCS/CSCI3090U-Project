#version 330 core

uniform mat4 u_MVP;
uniform mat4 u_MV;
uniform vec3 u_light_pos;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coords;
layout(location = 2) in vec3 texture_normal;

out vec2 UV;
out vec3 lightvec_view;
out vec3 norm_view;

void main(){
    // Set the position
    gl_Position = u_MVP * vec4(position, 1.0);

    // Calculate the lighting
    vec3 pos_viewspace = vec3(u_MV * vec4(position, 1.0));
    norm_view = normalize(vec3(u_MV * vec4(texture_normal, 0.0)));
    lightvec_view = normalize(u_light_pos - pos_viewspace);
    UV = texture_coords;
}
