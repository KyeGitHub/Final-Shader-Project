#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
//uniform mat4 matrixView; not used
uniform mat4 matrixModelView;

uniform float t;			// real time


layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out float reflFactor;



void main(void) 
{

	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	// no light calculation for water
	color = vec4(1, 1, 1, 1);

	// calculate reflection coefficient
	// using Schlick's approximation of Fresnel formula
	float cosTheta = dot(normal, normalize(-position.xyz));
	float R0 = 0.06;
	reflFactor = R0 + (1 - R0) * pow(1.0 - cosTheta, 5);

}
