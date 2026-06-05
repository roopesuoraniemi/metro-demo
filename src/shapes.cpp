#include "shapes.h"
#include "raylib.h"

void DrawDottedRectangle(Vector3 center, Vector3 size, Color color, float step) {
    for (float x = center.x - size.x/2.0f; x <= center.x + size.x/2.0f + 0.001f; x += step) {
        for (float y = center.y - size.y/2.0f; y <= center.y + size.y/2.0f + 0.001f; y += step) {
            for (float z = center.z - size.z/2.0f; z <= center.z + size.z/2.0f + 0.001f; z += step) {
                // Draw surface only to make it cleaner
                bool onSurface = (
                    x <= center.x - size.x/2.0f + step/2.0f || x >= center.x + size.x/2.0f - step/2.0f ||
                    y <= center.y - size.y/2.0f + step/2.0f || y >= center.y + size.y/2.0f - step/2.0f ||
                    z <= center.z - size.z/2.0f + step/2.0f || z >= center.z + size.z/2.0f - step/2.0f
                );
                if (onSurface) {
                    DrawPoint3D((Vector3){ x, y, z }, color);
                }
            }
        }
    }
}

void DrawFinnishMetro() {
    Vector3 center = { 50.0f, 2.0f, -25.0f };
    
    float carLength = 8.0f;
    float carWidth = 2.5f;
    float carHeight = 2.5f;
    float gap = 0.5f;
    float step = 0.2f;

    for (int i = 0; i < 2; i++) {
        Vector3 carPos = { 
            center.x + (i - 0.5f) * (carLength + gap), 
            center.y, 
            center.z 
        };
        
        // Orange body
        DrawDottedRectangle(carPos, (Vector3){carLength, carHeight, carWidth}, ORANGE, step);
        
        // Gray roof
        Vector3 roofPos = { carPos.x, carPos.y + carHeight/2.0f + 0.2f, carPos.z };
        DrawDottedRectangle(roofPos, (Vector3){carLength, 0.4f, carWidth}, LIGHTGRAY, step);
        
        // Dark gray windows
        Vector3 windowPos = { carPos.x, carPos.y + 0.3f, carPos.z };
        DrawDottedRectangle(windowPos, (Vector3){carLength - 1.0f, 0.8f, carWidth + 0.1f}, DARKGRAY, step);
    }
}
