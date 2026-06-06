#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // Compute MVP for current instance
    mat4 mvpi = mvp * instanceTransform;
    
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Calculate final vertex position
    gl_Position = mvpi * vec4(vertexPosition, 1.0);
}
