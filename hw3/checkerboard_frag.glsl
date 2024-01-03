#version 330 core

in vec4 fragPos; // Position of the fragment in world coordinates
in vec4 color;   // Color of the fragment

vec4 white = vec4(1.0, 1.0, 1.0, 1.0); // White color
vec4 black = vec4(0.0, 0.0, 0.0, 1.0); // Black color
int checkerboardSize = 150;              // Checkerboard size 
int offsetMultiplier = 1;             // Increase this value to increase the speed of the checkerboard movement

out vec4 fragColor;

uniform float offset; // Offset value
uniform vec3 scale;  // Scale value

void main() {
    // Calculate checkerboard pattern logic
    bool x = int((fragPos.x * 32 + checkerboardSize) * scale.x) % (2 * checkerboardSize) >= checkerboardSize;
    bool z = int((fragPos.z + offset * offsetMultiplier) * scale.z * 2) % (2 * checkerboardSize) >= checkerboardSize;
    if (x == z) { // Interpolate between black and white
        fragColor = color + black * 0.4;
    } else { // Interpolate between black and color
        fragColor = color + white * 0.4;
    }
}
