#version 330 core

// Texture Information
uniform bool u_lighting_switch;
uniform sampler2D u_texture_sampler;
uniform vec4 u_color;
uniform vec3 u_eye_pos;
uniform float u_shininess;

in vec2 UV;
in vec3 lightvec_view;
in vec3 norm_view;
in vec3 pos_view;
in vec3 light_pos;

out vec4 outColor;

void main(){
    float distance = length(light_pos - pos_view);
    // Ambient
    vec4 ambient_color = vec4(0.3, 0.3, 0.3, 1.0) * vec4(texture(u_texture_sampler, UV).rgb, 1.0);
    // Diffuse
    float diffuse = clamp(dot(norm_view, lightvec_view), 0, 1);
    // Attenuate
    diffuse = diffuse * (1.0 / (1.0 + (0.00025 * distance * distance)));
    // Specular
    vec3 incid_vec = -lightvec_view;
    vec3 refl_vec = reflect(incid_vec, norm_view);
    vec3 eye_vec = normalize(u_eye_pos - pos_view);
    float cos_angle = clamp(dot(eye_vec, refl_vec), 0, 1);
    float specular = pow(cos_angle, u_shininess);


    outColor = vec4(1.0, 1.0, 1.0, 1.0) * specular 
        + vec4(texture(u_texture_sampler, UV).rgb, 1.0) * diffuse 
        + ambient_color;
}
