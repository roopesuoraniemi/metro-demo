#version 330

// Input variables coming from Raylib's default vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;

// Uniforms provided by Raylib
uniform sampler2D texture0; // The RenderTexture we are passing in

// Output color for this pixel
out vec4 finalColor;

void main()
{
    // The base color of the current pixel
    // --- Chromatic Aberration (Lens Distortion) ---
    float aberrationAmount = 0.0015; // Distortion strength
    vec4 texColor;
    texColor.r = texture(texture0, fragTexCoord + vec2(aberrationAmount, 0.0)).r;
    texColor.g = texture(texture0, fragTexCoord).g;
    texColor.b = texture(texture0, fragTexCoord - vec2(aberrationAmount, 0.0)).b;
    texColor.a = texture(texture0, fragTexCoord).a;
    
    // --- Improved Single-Pass Blur (Gaussian 3x3) ---
    vec4 sum = vec4(0.0);
    
    // Adjusted blur size
    float blurSize = 0.008; 
    
    // 3x3 Gaussian kernel approximations
    sum += texture(texture0, fragTexCoord + vec2(-blurSize, -blurSize)) * 1.0;
    sum += texture(texture0, fragTexCoord + vec2(-blurSize, 0.0)) * 2.0;
    sum += texture(texture0, fragTexCoord + vec2(-blurSize, blurSize)) * 1.0;
    sum += texture(texture0, fragTexCoord + vec2(0.0, -blurSize)) * 2.0;
    sum += texture(texture0, fragTexCoord) * 4.0; // Center
    sum += texture(texture0, fragTexCoord + vec2(0.0, blurSize)) * 2.0;
    sum += texture(texture0, fragTexCoord + vec2(blurSize, -blurSize)) * 1.0;
    sum += texture(texture0, fragTexCoord + vec2(blurSize, 0.0)) * 2.0;
    sum += texture(texture0, fragTexCoord + vec2(blurSize, blurSize)) * 1.0;
    
    // Average based on the kernel weights
    sum = sum / 16.0;

    // --- Bright-Pass Filter (Hue Preserving) ---
    float brightness = max(max(sum.r, sum.g), sum.b);
    
    // Less aggressive threshold and intensity
    float contribution = max(brightness - 0.35, 0.0) / max(brightness, 0.0001);
    vec4 highlight = sum * contribution;
    
    // Standard intensity boost
    highlight *= 6.0;
    
    // Add the glowing highlights on top of the distorted, original render
    finalColor = texColor + highlight;
}
