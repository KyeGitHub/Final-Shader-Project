// FRAGMENT SHADER
#version 330

in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in vec3 texCoordCubeMap;
in vec4 shadowCoord;


out vec4 outColor;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform float reflectionPower;

// View Matrix
uniform mat4 matrixView;

// Texture
uniform sampler2D texture0;
uniform samplerCube textureCubeMap;
uniform sampler2DShadow shadowMap;

// Shadow Power
uniform float shadowPower = 1.0;

struct POINT
{
	int on;
	vec3 position;
	mat4 transform;
	vec3 diffuse;
	vec3 specular;
};

uniform POINT lightPoint1, lightPoint2, lightPoint3;

vec4 PointLight(POINT light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);

	// diffuse light
	vec3 L = normalize(light.transform * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	return color;
}

void main(void) 
{
	outColor = color;
	if (lightPoint1.on == 1) 
		outColor += PointLight(lightPoint1);
	if (lightPoint2.on == 1) 
		outColor += PointLight(lightPoint2);
	if (lightPoint3.on == 1) 
		outColor += PointLight(lightPoint3);

	outColor *= mix(texture(texture0, texCoord0.st), texture(textureCubeMap, texCoordCubeMap), reflectionPower);

	// Calculation of the shadow
	float shadow = 1.0;
	if (shadowCoord.w > 0)	// if shadowCoord.w < 0 fragment is out of the Light POV
			shadow = 0.5 + 0.5 * textureProj(shadowMap, shadowCoord);

	//float shadow = 1.0;
	//if (shadowCoord.w > 0)	// if shadowCoord.w < 0 fragment is out of the Light POV
	//{
	//	// Calculate perspective-divided coordinates (necessary for perspective projection)
	//	vec4 shadowCoordDiv = shadowCoord / shadowCoord.w;
	//	// Calculate the shadow depth (the distance from the light according to Shadow Map)
	//	float shadowDepth = texture2D(shadowMap, shadowCoordDiv.st).z;
	//	// Shadow only if the true depth (shadowCoordDiv.z) greater than shadow depth
	//	if (shadowCoordDiv.z > shadowDepth)
	//		shadow = 0.5;
	//}

	if (lightPoint3.on == 1){
		outColor *= shadow;
	}
	
	outColor *= texture(texture0, texCoord0);
}
