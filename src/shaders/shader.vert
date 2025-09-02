#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    float angle = radians(45.0);

    mat4 rotation = mat4(
        cos(angle), 0.0, sin(angle), 0.0,
        0.0,        1.0, 0.0,        0.0,
       -sin(angle), 0.0, cos(angle), 0.0,
        0.0,        0.0, 0.0,        1.0
    );

    mat4 translation = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, -2.5,
        0.0, 0.0, 0.0, 1.0
    );

    vec3 scaledPosition = inPosition * 0.2;
    gl_Position = translation * rotation * vec4(scaledPosition, 1.0);

    fragColor = vec3(1.0);
}