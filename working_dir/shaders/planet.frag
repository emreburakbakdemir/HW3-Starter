#version 430
/*
	File Name	: planet.frag
	Description	: Planet fragment shader with Phong lighting
*/

// Definitions
#define IN_UV			layout(location = 0)
#define IN_NORMAL		layout(location = 1)
#define IN_WORLD_POS	layout(location = 2)

#define OUT_FBO			layout(location = 0)

#define T_ALBEDO		layout(binding = 0)
#define T_SHADOW_MAP	layout(binding = 4)

#define U_LIGHT_DIR		layout(location = 4)
#define U_LIGHT_COLOR	layout(location = 5)
#define U_EYE_POS		layout(location = 6)
#define U_LIGHT_VP		layout(location = 7)
#define U_USE_SHADOWS	layout(location = 8)

// Input
in IN_UV		 vec2 fUV;
in IN_NORMAL	 vec3 fNormal;
in IN_WORLD_POS	 vec3 fWorldPos;

// Output
out OUT_FBO vec4 fboColor;

// Uniforms
U_LIGHT_DIR		uniform vec3 uLightDir;
U_LIGHT_COLOR	uniform vec3 uLightColor;
U_EYE_POS		uniform vec3 uEyePos;
U_LIGHT_VP		uniform mat4 uLightVP;
U_USE_SHADOWS	uniform int uUseShadows;

// Textures
uniform T_ALBEDO sampler2D tAlbedo;
uniform T_SHADOW_MAP sampler2D tShadowMap;

void main(void)
{
	// Sample albedo texture
	vec3 albedo = texture(tAlbedo, fUV).rgb;

	// Normalize interpolated normal
	vec3 N = normalize(fNormal);

	// Light direction (pointing towards light)
	vec3 L = normalize(-uLightDir);

	// View direction
	vec3 V = normalize(uEyePos - fWorldPos);

	// Half vector for Blinn-Phong
	vec3 H = normalize(L + V);

	// Ambient component
	vec3 ambient = 0.2 * albedo;

	// Shadow calculation
	float shadow = 0.0;
	if (uUseShadows > 0)
	{
		vec4 lightSpacePos = uLightVP * vec4(fWorldPos, 1.0);
		vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

		// Convert from NDC [-1,1] to texture coordinates [0,1]
		projCoords.xy = projCoords.xy * 0.5 + 0.5;

		float currentDepth = projCoords.z;
		float closestDepth = texture(tShadowMap, projCoords.xy).r;

		// Shadow bias to fix shadow acne
		float bias = 0.005;
		shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
	}

	// Diffuse component
	float diffuseTerm = max(dot(N, L), 0.0);
	vec3 diffuse = diffuseTerm * albedo * uLightColor * (1.0 - shadow);

	// Specular component
	float specularPower = 32.0;
	float specularTerm = pow(max(dot(N, H), 0.0), specularPower);
	vec3 specular = specularTerm * uLightColor * 0.5 * (1.0 - shadow);

	// Combine all components
	vec3 finalColor = ambient + diffuse + specular;

	fboColor = vec4(finalColor, 1.0);
}
