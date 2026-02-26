#version 450

// input from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

// output on screen
layout(location = 0) out vec4 outColor;

// bindings
layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

void main() {
    vec4 textureColor = texture(texSampler, fragTexCoord);
    vec3 lightDir = normalize(ubo.cameraPos - fragPos);
    float diff = max(dot(normalize(fragColor), lightDir), 0.1);
    vec3 finalColor = diff * textureColor.rgb;

    outColor = vec4(finalColor, 1.0);
}