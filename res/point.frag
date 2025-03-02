#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

uniform vec3 color;

uniform float size;

void main() {
    float radius = length(TexCoord - vec2(0.5, 0.5)) * 2.0;
    if (radius > 1.0)
        discard;

    FragColor = vec4(color, 1.0);
}