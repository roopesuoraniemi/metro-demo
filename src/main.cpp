#include "raylib.h"
#include "raymath.h"
#include "shapes.h"

int main() {

  const float camera_speed = 0.01f;
  const float turn_responsiveness = 2.8f;
  const float zoom_arc_height = 7.0f;
  const float zoom_duration = 2.0f;
  const float ride_duration = 1.0f;

  float metro_size = 1.0f;
  bool zoom_done = false;
  bool zoom_path_started = false  ;
  bool ride_path_started = false;
  float zoom_t = 2.0f;
  float ride_t = 2.0f;
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
  Vector3 zoom_start = camera.position;
  Vector3 zoom_end = camera.position;
  Vector3 ride_start = camera.position;
  Vector3 ride_end = camera.position;

  auto EaseInOut = [](float t) { return t * t * (3.0f - 2.0f * t); };

  auto FloorParallelArcPoint = [](Vector3 start, Vector3 end, float t, float arcWidth) {
    Vector3 p = Vector3Lerp(start, end, t);
    Vector3 flatDir = {end.x - start.x, 0.0f, end.z - start.z};
    float flatLen = Vector3Length(flatDir);
    if (flatLen > 0.0001f) {
      Vector3 perp = {-flatDir.z / flatLen, 0.0f, flatDir.x / flatLen};
      float sideOffset = 4.0f * arcWidth * t * (1.0f - t);
      p = Vector3Add(p, Vector3Scale(perp, sideOffset));
    }
    return p;
  };

  const Vector3 dotPosition = {50.0f, 5.0f, 0.0f};
  const float dotRadius = 0.12f;

  RenderTexture2D target =
      LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
  Shader bloomShader = LoadShader(0, "bloom.fs");

  InitMetro(metro_size);

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    if (frame < 200) {UpdateCamera(&camera, CAMERA_ORBITAL);}
    if (!zoom_done && frame > 200) {
      Vector3 door_location = GetMetroDoorLocation(2, true, metro_size);
      float distance = Vector3Distance(door_location, camera.position);
      float target_distance = Vector3Distance(camera.target, door_location);
      if (!zoom_path_started) {
        zoom_path_started = true;
        zoom_start = camera.position;
        zoom_end = door_location;
        zoom_t = 0.0f;
      }
      zoom_t += GetFrameTime() / zoom_duration;
      if (zoom_t > 1.0f) zoom_t = 1.0f;
      camera.position = FloorParallelArcPoint(zoom_start, zoom_end, EaseInOut(zoom_t), zoom_arc_height);
      if (zoom_t >= 1.0f || distance < 0.08f) {zoom_done = true;}
      if (target_distance >= 0.01f) {
        float turn_alpha = 1.0f - expf(-turn_responsiveness * GetFrameTime());
        camera.target = Vector3Lerp(camera.target, door_location, turn_alpha);
      }
    }

    if (zoom_done && frame > 250) {
      Vector3 center = GetMetroInsideLocation(2, metro_size);
      Vector3 target_location = GetMetroEndLocation(metro_size);
      float distance = Vector3Distance(center, camera.position);
      float target_distance = Vector3Distance(camera.target, target_location);
      if (!ride_path_started) {
        ride_path_started = true;
        ride_start = camera.position;
        ride_end = center;
        ride_t = 0.0f;
      }
      if (distance >= 0.3f && ride_t < 1.0f) {
        ride_t += GetFrameTime() / ride_duration;
        if (ride_t > 1.0f) ride_t = 1.0f;
        camera.position = Vector3Lerp(ride_start, ride_end, EaseInOut(ride_t));
      }
      if (target_distance >= 0.001f) {
        float turn_alpha = 1.0f - expf(-turn_responsiveness * GetFrameTime());
        camera.target = Vector3Lerp(camera.target, target_location, turn_alpha);
      }
    }

    metro_size = 1.0f + 0.5f * sinf((frame - 350) * 0.05f) * sinf((frame - 350) * 0.05f);

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

    DrawTextureRec(target.texture,
                   (Rectangle){0, 0, (float)target.texture.width,
                               (float)-target.texture.height},
                   (Vector2){0, 0}, WHITE);
    EndShaderMode();

    DrawText("Raylib is working!!", 20, 20, 24, LIGHTGRAY);
    DrawText("Wubbalubbadubdub", 20, 52, 18, GRAY);

    EndDrawing();
    frame++;
  }

  UnloadMetro(); // Clean up generated models and vectors
  UnloadShader(bloomShader);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
