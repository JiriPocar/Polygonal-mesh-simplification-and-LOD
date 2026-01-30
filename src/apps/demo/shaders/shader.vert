#version 450

// vertex attributes (per-vertex, binding 0)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// instance attributes (per-instance, binding 1)
layout(location = 3) in vec3 instancePosition;
layout(location = 4) in uint instanceModelType;
layout(location = 5) in uint instanceLodLevel;

// outputs to fragment shader
layout(location = 0) out flat vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// ubo
layout(binding = 0) uniform UBO {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

// push constants (LOD to draw)
layout(push_constant) uniform PushConstants {
	uint currentLOD;
} push;

void main()
{
	// filter out instances with different LOD than current
	if(instanceLodLevel != push.currentLOD)
	{
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
		return;
	}

	// create model matrix from instance positions
	mat4 instanceModel = mat4(1.0);
	instanceModel[3] = vec4(instancePosition, 1.0);

	

	// transform vertex position
	gl_Position = ubo.proj * ubo.view * instanceModel * vec4(inPosition, 1.0);

	// diffuse lightning
	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	float diff = max(dot(inNormal, lightDir), 0.2);

	vec3 lodColors[4] = vec3[4](
		vec3(0.2, 1.0, 0.2),  // LOD0 - green
		vec3(1.0, 1.0, 0.2),  // LOD1 - yellow
		vec3(1.0, 0.5, 0.2),  // LOD2 - orange
		vec3(1.0, 0.2, 0.2)   // LOD3 - red
    );

	vec3 baseColor = lodColors[instanceLodLevel];
	fragColor = baseColor * diff;

	fragTexCoord = inTexCoord;
}