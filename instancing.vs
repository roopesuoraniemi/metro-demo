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
uniform mat4 metroTransform;

// Snake undulation: lateral sine wave traveling along the train length.
uniform vec3 metroCenter;
uniform float waveAmp;
uniform float waveK;
uniform float waveOmega;
uniform float waveTime;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // Baked dot position (before the per-metro transform). Offset it laterally
    // by a sine of its distance along the train, so the body squiggles.
    vec4 p0 = instanceTransform * vec4(vertexPosition, 1.0);
    float s = p0.x - metroCenter.x;
    p0.z += waveAmp * sin(waveK * s - waveOmega * waveTime);

    gl_Position = mvp * metroTransform * p0;
}
