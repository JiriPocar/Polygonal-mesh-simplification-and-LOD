#version 450

// vertex attributes (per-vertex, binding 0)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// instance attributes (per-instance, binding 1)
layout(location = 3) in vec3 instancePosition;
layout(location = 4) in uint instanceLodLevel;

// outputs to fragment shader
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out uint fragLodLevel;

// ubo
layout(binding = 0) uniform UBO {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
	uint showLodColors;
} ubo;

void main()
{
	float scale = 4;
	vec3 worldPos = (inPosition * scale) + instancePosition;

	gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);

	fragColor = inNormal;
	fragTexCoord = inTexCoord;
	fragLodLevel = instanceLodLevel;
}