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

uniform float disintegrateAmount;

float hash(vec3 p) {
    p = fract(p * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.x + p.y) * p.z);
}

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

    // Disintegration displacement based on instance local position
    vec3 localPos = vec3(instanceTransform[3][0], instanceTransform[3][1], instanceTransform[3][2]);
    vec3 randDir = vec3(
        hash(localPos) * 2.0 - 1.0,
        hash(localPos + vec3(1.0)) * 2.0 - 1.0,
        hash(localPos + vec3(2.0)) * 2.0 - 1.0
    );
    randDir = normalize(randDir);
    
    float speed = hash(localPos + vec3(3.0)) * 20.0 + 5.0;
    vec3 displacement = randDir * speed * disintegrateAmount;
    displacement.y -= disintegrateAmount * disintegrateAmount * 15.0; // gravity
    
    p0.xyz += displacement;

    gl_Position = mvp * metroTransform * p0;
}
