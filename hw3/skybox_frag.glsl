#version 330 core

// Input texture coordinates
in vec2 TexCoord;

// Output color
out vec4 FragColor;

// Uniform sampler for the background image
uniform sampler2D backgroundTexture;

void main()
{
    // Reverse the texture coordinates horizontally
    vec2 flippedTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);

    // Sample the background texture using the flipped texture coordinates
    vec4 background = texture(backgroundTexture, flippedTexCoord);

    // Output the sampled color as the final color
    FragColor = background;
}
