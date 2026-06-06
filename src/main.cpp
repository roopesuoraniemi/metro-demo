#include "raylib.h"
#include "shapes.h"
#include "raymath.h"

int main() {

    const float camera_speed = 0.03f;
    const float turn_speed = 0.1f;

    int frame = 0;

    InitWindow(0, 0, "raylib demoscene starter");
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    DisableCursor();

    Camera3D camera = {0};
    camera.position = (Vector3){50.0f, 0.0f, -35.0f};
    camera.target = (Vector3){50.0f, 5.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const Vector3 dotPosition = {50.0f, 5.0f, 0.0f};
    const float dotRadius = 0.12f;

    RenderTexture2D target = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    Shader bloomShader = LoadShader(0, "bloom.fs");


    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (frame < 200) {UpdateCamera(&camera, CAMERA_ORBITAL);}
        if (frame > 200) {
            Vector3 door_location = GetMetroDoorLocation(2, false);
            float distance = Vector3Distance(door_location, camera.position);
            float target_distance = Vector3Distance(camera.target, door_location);
            if (distance >= 0.5f) {
                camera.position = Vector3Lerp(camera.position, door_location, camera_speed);
            }
            if (target_distance >= 1.0f) {
                camera.target = Vector3Lerp(camera.target, door_location, turn_speed);
            }


        }

        BeginTextureMode(target);
          ClearBackground(BLACK);

          BeginMode3D(camera);
              DrawSphere(dotPosition, dotRadius, RAYWHITE);
              DrawFinnishMetro();
          EndMode3D();
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);

            BeginShaderMode(bloomShader);

                DrawTextureRec(
                    target.texture,
                    (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                    (Vector2){ 0, 0 },
                    WHITE
                );
            EndShaderMode();

            DrawText("Raylib is working!!", 20, 20, 24, LIGHTGRAY);
            DrawText("Wubbalubbadubdub", 20, 52, 18, GRAY);

        EndDrawing();
        frame++;
    }

    UnloadShader(bloomShader);
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
