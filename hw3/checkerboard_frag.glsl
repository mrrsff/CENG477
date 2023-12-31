#version 330 core

in vec4 fragPos; // Position of the fragment in world coordinates
in vec4 color;   // Color of the fragment

vec4 white = vec4(1.0, 1.0, 1.0, 1.0); // White color
int checkerboardSize = 4;              // Checkerboard size

out vec4 fragColor;

uniform float offset; // Offset value
uniform vec3 scale;  // Scale value

void main() {
    // Calculate checkerboard pattern logic
    bool x = int(fragPos.x * scale.x) % (2 * checkerboardSize) >= checkerboardSize;
    bool z = int((fragPos.z + offset) * scale.z) % (2 * checkerboardSize) >= checkerboardSize;

    // Assign colors based on the checkerboard pattern
    if (x != z) {
        fragColor = color - white * 0.2;
    } else { // Interpolate between black and color
        fragColor = color + white * 0.2;
    }
}
