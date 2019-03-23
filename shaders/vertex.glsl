#version 330 core

uniform mat4 u_MVP;
uniform mat4 u_MV;
uniform vec4 u_color;
uniform vec3 u_light_pos;
uniform bool u_texture_switch;

// Texture sampler definition
uniform sampler2D u_texture_sampler;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coords;
layout(location = 2) in vec3 texture_normal;

out vec4 v_color;

void main(){
    // Set the position
    gl_Position = u_MVP * vec4(position, 1.0);

    // Calculate the lighting
    vec3 pos_viewspace = vec3(u_MV * vec4(position, 1.0));
    vec3 norm_viewspace = normalize(vec3(u_MV * vec4(texture_normal, 0.0)));;

    float distance = length(u_light_pos - pos_viewspace);
    vec3 lightvector_viewspace = normalize(u_light_pos - pos_viewspace);

    // Textures
    vec2 tex_color = texture_coords;
    vec3 tex_norm = texture_normal;
    
    // Ambient Light
    vec4 ambient_color;
    if(u_texture_switch){
        ambient_color = vec4(0.1, 0.1, 0.1, 1.0) * vec4(texture(u_texture_sampler, tex_color).rgb, 1.0);
    } else {
        ambient_color = vec4(0.1, 0.1, 0.1, 1.0) * u_color;
    }

    // Diffuse
    float diffuse = clamp(dot(norm_viewspace, lightvector_viewspace), 0, 1);

    // Set the color for the fragment shader
    if(u_texture_switch){
        v_color = vec4(texture(u_texture_sampler, tex_color).rgb, 1.0) * diffuse + ambient_color;
    } else {
        v_color = u_color * diffuse + ambient_color;
    }
}
