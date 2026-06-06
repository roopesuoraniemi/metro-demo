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
    vec3 localPos = vec3(instanceTransform[3][0], instanceTransform[3][1], instanceTransform[3][2]);
    
    vec3 randDir = vec3(
        hash(localPos) * 2.0 - 1.0,
        hash(localPos + vec3(1.0)) * 2.0 - 1.0,
        hash(localPos + vec3(2.0)) * 2.0 - 1.0
    );
    randDir = normalize(randDir);
    
    float speed = hash(localPos + vec3(3.0)) * 20.0 + 5.0;
    vec3 displacement = randDir * speed * disintegrateAmount;
    
    // Adding some gravity as it disintegrates more
    displacement.y -= disintegrateAmount * disintegrateAmount * 15.0;
    
    mat4 modifiedInstance = instanceTransform;
    modifiedInstance[3][0] += displacement.x;
    modifiedInstance[3][1] += displacement.y;
    modifiedInstance[3][2] += displacement.z;

    mat4 mvpi = mvp * metroTransform * modifiedInstance;
    
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    gl_Position = mvpi * vec4(vertexPosition, 1.0);
}
