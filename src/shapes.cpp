#include "shapes.h"
#include "raylib.h"

void DrawMetroCarSurface(Vector3 center, float length, float height, float width, float step, float dotRadius) {
    float minX = center.x - length/2.0f;
    float maxX = center.x + length/2.0f;
    float minY = center.y - height/2.0f;
    float maxY = center.y + height/2.0f;
    float minZ = center.z - width/2.0f;
    float maxZ = center.z + width/2.0f;

    auto getColor = [&](Vector3 p) -> Color {
        // Roof is light gray
        if (p.y >= maxY - step/2.0f) return LIGHTGRAY;

        // Sides check for windows and doors
        if (p.z <= minZ + step/2.0f || p.z >= maxZ - step/2.0f) {
            float relX = p.x - center.x;
            float relY = p.y - center.y;
            
            // Doors (1 in the middle)
            float doorOffset = 0.0f;
            if (relX >= doorOffset - 0.5f && relX <= doorOffset + 0.5f &&
                relY >= -1.1f && relY <= 0.7f) {
                return GRAY;
            }
            
            // Windows (2 on each side)
            float windowOffsets[4] = {-3.5f, -1.8f, 1.8f, 3.5f};
            for (int w = 0; w < 4; w++) {
                if (relX >= windowOffsets[w] - 0.6f && relX <= windowOffsets[w] + 0.6f &&
                    relY >= -0.1f && relY <= 0.7f) {
                    return BLUE;
                }
            }
        }
        
        // Main body is orange
        return ORANGE;
    };

    // Draw Top and Bottom faces
    for (float x = minX; x <= maxX + 0.001f; x += step) {
        for (float z = minZ; z <= maxZ + 0.001f; z += step) {
            DrawCube((Vector3){ x, maxY, z }, dotRadius*2.0f, dotRadius*2.0f, dotRadius*2.0f, getColor((Vector3){ x, maxY, z }));
            DrawCube((Vector3){ x, minY, z }, dotRadius*2.0f, dotRadius*2.0f, dotRadius*2.0f, getColor((Vector3){ x, minY, z }));
        }
    }

    // Draw Front and Back faces (skipping top/bottom edges)
    for (float x = minX; x <= maxX + 0.001f; x += step) {
        for (float y = minY + step; y <= maxY - step + 0.001f; y += step) {
            DrawCube((Vector3){ x, y, maxZ }, dotRadius*2.0f, dotRadius*2.0f, dotRadius*2.0f, getColor((Vector3){ x, y, maxZ }));
            DrawCube((Vector3){ x, y, minZ }, dotRadius*2.0f, dotRadius*2.0f, dotRadius*2.0f, getColor((Vector3){ x, y, minZ }));
        }
    }

    // Draw Left and Right faces (skipping outer edges)
    for (float y = minY + step; y <= maxY - step + 0.001f; y += step) {
        for (float z = minZ + step; z <= maxZ - step + 0.001f; z += step) {
            DrawCube((Vector3){ minX, y, z }, dotRadius*2.0f, dotRadius*2.0f, dotRadius*2.0f, getColor((Vector3){ minX, y, z }));
            DrawCube((Vector3){ maxX, y, z }, dotRadius*2.0f, dotRadius*2.0f, dotRadius*2.0f, getColor((Vector3){ maxX, y, z }));
        }
    }
}

void DrawFinnishMetro() {
    Vector3 center = { 50.0f, 2.0f, -25.0f };
    
    int numCars = 4;
    float carLength = 10.0f;
    float carWidth = 2.5f;
    float carHeight = 2.5f;
    float gap = 0.5f;
    
    float step = 0.4f;
    float dotRadius = 0.08f;

    for (int i = 0; i < numCars; i++) {
        Vector3 carPos = { 
            center.x + (i - (numCars - 1) / 2.0f) * (carLength + gap), 
            center.y, 
            center.z 
        };
        
        DrawMetroCarSurface(carPos, carLength, carHeight, carWidth, step, dotRadius);
    }
}

// Returns the center position of a door on the specified metro car.
Vector3 GetMetroDoorLocation(int carIndex, bool side) {
    Vector3 trainCenter = { 50.0f, 2.0f, -25.0f };
    int numCars = 4;
    float carLength = 10.0f;
    float carWidth = 2.5f;
    float gap = 0.5f;

    if (carIndex < 0) carIndex = 0;
    if (carIndex > numCars - 1) carIndex = numCars - 1;

    float doorX = trainCenter.x + (carIndex - (numCars - 1) / 2.0f) * (carLength + gap);;

    //Calculate Door Y
    float doorY = trainCenter.y - 0.2f;

    // Calculate Door Z
    float doorZ = trainCenter.z + (side ? (carWidth / 2.0f) : -(carWidth / 2.0f));

    return (Vector3){ doorX, doorY, doorZ };
}