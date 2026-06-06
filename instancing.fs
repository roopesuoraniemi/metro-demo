#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform vec4 colDiffuse;

// Output color
out vec4 finalColor;

void main()
{
    finalColor = colDiffuse;
}
