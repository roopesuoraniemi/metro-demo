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
    vec4 texColor = texture(texture0, fragTexCoord);
    
    // --- Crude Single-Pass Blur ---
    // We sample surrounding pixels to create a "spread" effect.
    vec4 sum = vec4(0.0);
    
    // The distance to sample. Increase this for a wider, softer glow.
    // (In a real production shader, you'd pass screen resolution in as a uniform to scale this perfectly)
    float blurSize = 0.005; 
    
    // Sample a 3x3 grid around our current pixel
    sum += texture(texture0, fragTexCoord + vec2(-blurSize, -blurSize));
    sum += texture(texture0, fragTexCoord + vec2(-blurSize, 0.0));
    sum += texture(texture0, fragTexCoord + vec2(-blurSize, blurSize));
    sum += texture(texture0, fragTexCoord + vec2(0.0, -blurSize));
    sum += texture(texture0, fragTexCoord); // Center (our actual pixel)
    sum += texture(texture0, fragTexCoord + vec2(0.0, blurSize));
    sum += texture(texture0, fragTexCoord + vec2(blurSize, -blurSize));
    sum += texture(texture0, fragTexCoord + vec2(blurSize, 0.0));
    sum += texture(texture0, fragTexCoord + vec2(blurSize, blurSize));
    
    // Average the samples
    sum = sum / 9.0;
    
    // --- Bright-Pass Filter ---
    // We only want bright things (like your RAYWHITE sphere) to glow.
    // We subtract 0.5 (threshold) so dark areas become <= 0, then clamp it.
    vec4 highlight = clamp(sum - 0.5, 0.0, 1.0);
    
    // Boost the intensity of the isolated bright spots
    highlight *= 2.5; 

    // Add the glowing highlights on top of the crisp, original render
    finalColor = texColor + highlight;
}
