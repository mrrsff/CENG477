#version 330 core

in vec4 fragPos; // Position of the fragment in world coordinates
in vec4 color;   // Color of the fragment

vec4 white = vec4(1.0, 1.0, 1.0, 1.0); // White color
vec4 black = vec4(0.0, 0.0, 0.0, 1.0); // Black color
<<<<<<< HEAD
int checkerboardSize = 150;              // Checkerboard size 
=======
int checkerboardSize = 50;              // Checkerboard size 
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
int offsetMultiplier = 1;             // Increase this value to increase the speed of the checkerboard movement

out vec4 fragColor;

uniform float offset; // Offset value
uniform vec3 scale;  // Scale value

void main() {
    // Calculate checkerboard pattern logic
<<<<<<< HEAD
    bool x = int((fragPos.x * 32 + checkerboardSize) * scale.x) % (2 * checkerboardSize) >= checkerboardSize;
    bool z = int((fragPos.z + offset * offsetMultiplier) * scale.z * 2) % (2 * checkerboardSize) >= checkerboardSize;
=======
    bool x = int((fragPos.x * 16) * scale.x) % (2 * checkerboardSize) >= checkerboardSize;
    bool z = int((fragPos.z + offset * offsetMultiplier) * scale.z) % (2 * checkerboardSize) >= checkerboardSize;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d

    // Assign colors based on the checkerboard pattern
    if (x != z) {
        fragColor = black;
    } else { // Interpolate between black and color
        fragColor = color + white * 0.4;
    }
}
