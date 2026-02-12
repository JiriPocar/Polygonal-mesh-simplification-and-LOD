#version 450

// input from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

// output on screen
layout(location = 0) out vec4 outColor;

// bindings
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 textureColor = texture(texSampler, fragTexCoord);

    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(fragColor), lightDir), 0.2);
    vec3 finalColor = diff * textureColor.rgb;

    outColor = vec4(finalColor, 1.0);
}