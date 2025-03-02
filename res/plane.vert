#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 position;
uniform vec3 normal;
uniform float size;

void main() {
    Normal = aNormal;
    TexCoord = aTexCoord;

    vec3 n = normalize(normal);
    vec3 dx;
    vec3 dy;
    if (abs(dot(n, vec3(1.0, 0.0, 0.0))) == 1.0) {
        dx = normalize(cross(n, vec3(0.0, 1.0, 0.0)));
    } else {
        dx = normalize(cross(n, vec3(1.0, 0.0, 0.0)));
    }
    dy = normalize(cross(n, dx));

    vec3 pos = dx * aPos.x + dy * aPos.y;

    gl_Position = projection * view * vec4(position + pos * size, 1.0);
}
