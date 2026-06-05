#include "raylib.h"
#include "shapes.h"

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib demoscene starter");
    ToggleFullscreen();
    DisableCursor();

    Camera3D camera = {0};
    camera.position = (Vector3){50.0f, 0.0f, -35.0f};
    camera.target = (Vector3){50.0f, 5.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const Vector3 dotPosition = {50.0f, 5.0f, 0.0f};
    const float dotRadius = 0.12f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        DrawSphere(dotPosition, dotRadius, RAYWHITE);
        DrawFinnishMetro();
        DrawGrid(20, 1.0f);
        EndMode3D();

        DrawText("Raylib is working!!", 20, 20, 24, LIGHTGRAY);
        DrawText("Wubbalubbadubdub", 20, 52, 18, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
