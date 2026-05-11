#version 450

// input from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in uint fragLodLevel;

// output on screen
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
    vec3 cameraPos;
    uint showLodColors;
} ubo;

// bindings
layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    if(ubo.showLodColors == 1)
    {
        vec3 lodColors[4] = vec3[](
			vec3(0.0, 1.0, 0.0), // LOD 0 - green
            vec3(1.0, 1.0, 0.0), // LOD 1 - yellow
			vec3(0.0, 0.5, 1.0), // LOD 2 - blue
            vec3(1.0, 0.0, 0.0)  // LOD 3 - red
		);
		
		uint LODNum = clamp(fragLodLevel, 0, 3);
		outColor = vec4(lodColors[LODNum], 1.0);
    }
    else
    {
        vec4 textureColor = texture(texSampler, fragTexCoord);
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        float diff = max(dot(normalize(fragColor), lightDir), 0.2);
        vec3 finalColor = diff * textureColor.rgb;
        outColor = vec4(finalColor, 1.0);
    }
}