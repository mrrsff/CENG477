#version 330 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them 
// here for simplicity.

vec3 I = vec3(1, 1, 1);          // point light intensity
vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 kd = vec3(0,0,0);     // diffuse reflectance coefficient
vec3 ka = vec3(0, 0, 1);   // ambient reflectance coefficient
vec3 ks = vec3(0,0,0);   // specular reflectance coefficient
vec3 lightPos = vec3(5, 5, 5);   // light position in world coordinates

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec3 eyePos;

layout(location=0) in vec3 inVertex; // This variable will contain the vertex position attribute from the vertex shader.
layout(location=1) in vec3 inNormal; // This variable will contain the vertex normal attribute from the vertex shader.

out vec4 color;
out vec4 fragPos;

void main(void)
{	 
	vec4 pWorld = modelingMatrix * vec4(inVertex, 1);

	vec3 ambientColor = Iamb * ka;

	color = vec4(ambientColor, 1);

    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);

	fragPos = pWorld;
}

