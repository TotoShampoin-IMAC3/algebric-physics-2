#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

uniform vec3 color;

uniform float size;

void main() {
    vec2 coord = abs((TexCoord - vec2(0.5, 0.5)) * 2.0 * size);

    float dx = coord.x - floor(coord.x);
    float dy = coord.y - floor(coord.y);

    float a =
        mix(0.5, 0.15, smoothstep(0.02, 0.03, dx) * smoothstep(0.02, 0.03, dy));

    vec3 c = color;
    if (!gl_FrontFacing) {
        c *= 0.25f;
    }

    FragColor = vec4(c, a);
}