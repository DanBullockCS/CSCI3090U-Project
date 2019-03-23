#version 330 core

out vec4 outColor;
in vec4 v_color;

// Texture sampler definition
uniform sampler2D textureSampler;
in vec3 tex_norm;
in vec2 tex_color;

void main(){
    outColor = v_color;

    // Textures
    //outColor = vec4(texture(textureSampler, tex_color).rgb, 1.0);
}
