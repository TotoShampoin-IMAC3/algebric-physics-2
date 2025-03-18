#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aStart;
layout(location = 4) in vec3 aEnd;

out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;

uniform float width;
// uniform vec3 start;
// uniform vec3 end;

void main() {
    vec3 start = aStart;
    vec3 end = aEnd;

    vec3 C = (start + end) / 2.0;
    float w = width / 2.0;
    float l = distance(start, end);
    vec3 vd = (inverse(view) * vec4(0.0, 0.0, -1.0, 0.0)).xyz;
    vec3 d = normalize(end - start);
    vec3 n = normalize(cross(vd, d));

    vec3 position = C + aPos.x * d * l / 2.0 + aPos.y * n * w;
    Normal = aNormal;
    TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(position, 1.0);
}