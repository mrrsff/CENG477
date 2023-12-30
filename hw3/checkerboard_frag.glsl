#version 330 core

in vec3 fragPos; // Position of the fragment in world coordinates

out vec4 fragColor;

uniform vec3 offset; // Offset value
uniform vec3 scale;  // Scale value

void main() {
    // Calculate checkerboard pattern logic
    bool x = bool(int((fragPos.x + offset.x) * scale.x) % 2 == 0);
    bool y = bool(int((fragPos.y + offset.y) * scale.y) % 2 == 0);
    bool z = bool(int((fragPos.z + offset.z) * scale.z) % 2 == 0);
    bool xorXY = x != y;

    // Assign colors based on the checkerboard pattern
    if (xorXY != z) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black color
    } else {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
    }
}
