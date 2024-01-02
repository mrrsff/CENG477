#version 330 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec3 eyePos;

// Input vertex position
in vec3 aPos;

// Output vertex position
out vec2 TexCoord;

void main()
{
    // Set the vertex position
    gl_Position = vec4(aPos, 1.0);

    // Map the vertex position to the texture coordinates
    TexCoord = aPos.xy * 0.5 + 0.5; // Assuming aPos is in the range [-1, 1]
}
