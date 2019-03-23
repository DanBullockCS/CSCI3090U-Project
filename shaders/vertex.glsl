#version 330 core

uniform mat4 u_MVP;
uniform mat4 u_MV;
uniform vec4 u_color;
uniform vec3 u_light_pos;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coords;
layout(location = 2) in vec3 texture_normal;

out vec4 v_color;

void main(){
    gl_Position = u_MVP * vec4(position, 1.0);

    // Calculate the lighting
    vec3 pos_viewspace = vec3(u_MV * vec4(position, 1.0));
    vec3 norm_viewspace = normalize(vec3(u_MV * vec4(texture_normal, 0.0)));;

    float distance = length(u_light_pos - pos_viewspace);
    vec3 lightvector_viewspace = normalize(u_light_pos - pos_viewspace);

    // Ambient Light
    vec4 ambient_color = vec4(0.1, 0.1, 0.1, 1.0) * u_color;

    // Diffuse
    float diffuse = clamp(dot(norm_viewspace, lightvector_viewspace), 0, 1);


    v_color = u_color * diffuse + ambient_color;
}
