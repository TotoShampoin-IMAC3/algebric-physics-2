#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aPoint;

out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;

// uniform vec3 point;
uniform float size;

void main() {
    vec3 position = aPoint;
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]) * size;
    vec3 up = vec3(view[0][1], view[1][1], view[2][1]) * size;

    Normal = aNormal;
    TexCoord = aTexCoord;

    gl_Position =
        projection * view * vec4(position + aPos.x * right + aPos.y * up, 1.0);
}
