#version 430
/*
	File Name	: earth.frag
	Description	: Earth-specific fragment shader with special effects and shadows
*/

// Definitions
#define IN_UV			layout(location = 0)
#define IN_NORMAL		layout(location = 1)
#define IN_WORLD_POS	layout(location = 2)

#define OUT_FBO			layout(location = 0)

#define T_ALBEDO		layout(binding = 0)
#define T_SPECULAR		layout(binding = 1)
#define T_NIGHT			layout(binding = 2)
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
uniform T_SPECULAR sampler2D tSpecular;
uniform T_NIGHT sampler2D tNight;
uniform T_SHADOW_MAP sampler2D tShadowMap;

void main(void)
{
	// Sample textures
	vec3 albedo = texture(tAlbedo, fUV).rgb;
	float specularMask = texture(tSpecular, fUV).r;
	vec3 nightLights = texture(tNight, fUV).rgb;

	// Normalize interpolated normal
	vec3 N = normalize(fNormal);

	// Light direction (pointing towards light)
	vec3 L = normalize(-uLightDir);

	// View direction
	vec3 V = normalize(uEyePos - fWorldPos);

	// Half vector for Blinn-Phong
	vec3 H = normalize(L + V);

	// Ambient component
	vec3 ambient = 0.4 * albedo;

	// Shadow calculation
	float shadow = 0.0;
	if (uUseShadows > 0)
	{
		vec4 lightSpacePos = uLightVP * vec4(fWorldPos, 1.0);
		vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
		projCoords.xy = projCoords.xy * 0.5 + 0.5;
		float currentDepth = projCoords.z;
		float closestDepth = texture(tShadowMap, projCoords.xy).r;
		float bias = 0.005;
		shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
	}

	// Diffuse component
	float diffuseTerm = max(dot(N, L), 0.0);
	vec3 diffuse = diffuseTerm * albedo * uLightColor * (1.0 - shadow);

	// Specular component with specular map
	float specularPowerLow = 8.0;
	float specularPowerHigh = 64.0;
	float specularPower = mix(specularPowerLow, specularPowerHigh, specularMask);
	float specularIntensity = mix(0.1, 0.8, specularMask);

	float specularTerm = pow(max(dot(N, H), 0.0), specularPower);
	vec3 specular = specularTerm * uLightColor * specularIntensity * (1.0 - shadow);

	// Night map blending
	float nightBlend = 1.0 - smoothstep(-0.1, 0.2, diffuseTerm);
	vec3 nightEmission = nightLights * nightBlend;

	// Combine all components
	vec3 finalColor = ambient + diffuse + specular + nightEmission;

	fboColor = vec4(finalColor, 1.0);
}
