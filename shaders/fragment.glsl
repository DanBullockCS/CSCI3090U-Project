#version 330 core

// Texture Information
uniform bool u_lighting_switch;
uniform sampler2D u_texture_sampler;
uniform vec4 u_color;

in vec2 UV;
in vec3 lightvec_view;
in vec3 norm_view;

out vec4 outColor;

void main(){
    vec4 ambient_color;
    float diffuse;
    if(u_lighting_switch){
        ambient_color = vec4(0.1, 0.1, 0.1, 1.0) * vec4(texture(u_texture_sampler, UV).rgb, 1.0);
        diffuse = clamp(dot(norm_view, lightvec_view), 0, 1);
        outColor = vec4(texture(u_texture_sampler, UV).rgb, 1.0) * diffuse + ambient_color;
    } else {
        outColor = vec4(texture(u_texture_sampler, UV).rgb, 1.0);
    }
}
