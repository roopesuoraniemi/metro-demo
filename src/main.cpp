#include "raylib.h"
#include "raymath.h"
#include "shapes.h"
#include <vector>

struct FallingCity {
  const char *name;
  float x;
  float y;
  int fontSize;
};

int main() {

  const float camera_speed = 0.01f;
  const float turn_responsiveness = 2.8f;
  const float zoom_arc_height = 7.0f;
  const float zoom_duration = 2.0f;
  const float ride_duration = 1.0f;
  const float return_duration = 6.0f;

  const float fly_initial_speed = 0.04f;
  const float fly_accel = 0.22f;

  float metro_size = 1.0f;
  bool zoom_done = false;
  bool zoom_path_started = false;
  bool ride_path_started = false;
  bool ride_done = false;
  bool fly_started = false;
  bool return_path_started = false;
  float zoom_t = 4.0f;
  float ride_t = 2.0f;
  float fly_progress = 0.0f;
  float fly_speed = 0.0f;
  float return_t = 0.0f;

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
  Vector3 return_start = camera.position;
  Vector3 return_focus_start = camera.target;

  auto EaseInOut = [](float t) { return t * t * (3.0f - 2.0f * t); };

  auto FloorParallelArcPoint = [](Vector3 start, Vector3 end, float t,
                                  float arcWidth) {
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

  const int virtualWidth = 1920;
  const int virtualHeight = 1080;
  RenderTexture2D target = LoadRenderTexture(virtualWidth, virtualHeight);
  Shader bloomShader = LoadShader(0, "bloom.fs");

  InitMetroResources();

  // Stationary metro that drives the camera sequence.
  Metro primary = CreateMetro((Vector3){50.0f, 2.0f, -25.0f}, 6, metro_size,
                              (Vector3){0.0f, 0.0f, 0.0f});

  // Additional metros are spawned only after the camera returns (see below),
  // so the intro shows the primary metro alone.
  std::vector<Metro> metros;

  // Star convergence: once the camera returns, all 6 metros are arranged
  // symmetrically around a center point and move inward like a collapsing star.
  bool star_started = false;
  bool spin_started = false;
  float spin_t = 0.0f;
  Vector3 spin_pivot = {0};
  float spin_start_angle = 0.0f;
  const int star_extra = 5;         // extra metros that fly in (primary stays)
  const float star_radius = 160.0f; // large enough to start off screen
  const float star_speed = 18.0f;
  const float star_prewarm_at =
      0.75f; // build star metros before fly-through ends
  bool star_prewarmed = false;
  std::vector<Metro> pendingStarMetros;
  const float spin_duration = 4.0f;   // time for the 360-degree spin
  const float spin_look_dist = 30.0f; // look target distance during the spin
  const float ride_out_speed = 55.0f; // original train rides out of frame

  // Cheap copies of the finished flower. The real metro flower is snapshotted
  // into a texture once, then drawn on flat ground quads so we can render many
  // of them for almost no cost (one draw call each instead of ~100k cubes).
  bool flowers_spawned = false;
  float bloom_zoom_t = 0.0f;
  Vector3 bloom_zoom_start = {0};
  std::vector<Vector3> flowerPositions;
  const int flower_grid = 4;           // (2*4+1)^2 - 1 = 80 cheap flowers
  const float flower_spacing = 130.0f; // distance between cheap flowers
  const float flower_size = 80.0f;     // world size of each flower quad
  const float bloom_zoom_duration = 6.0f;
  RenderTexture2D flowerRT = LoadRenderTexture(512, 512);
  Model flowerPlane =
      LoadModelFromMesh(GenMeshPlane(flower_size, flower_size, 1, 1));

  // After the zoom out, zoom back in, then drop the camera inside one flower
  // metro and ride it as it drives away on a curve. Cheap flowers and the
  // leftover metros are culled once they are off screen.
  bool zoomin_started = false;
  bool flowers_cleared = false;
  bool attached = false;
  float zoomin_t = 0.0f;
  Vector3 zoomin_start = {0};
  Vector3 zoomin_target_start = {0};
  Vector3 follow_pose = {0};
  Vector3 follow_target = {0};
  Metro chosenMetro; // the metro the camera rides once attached
  float drive_heading = 0.0f;
  const int chosen_idx = 0;
  const float zoomin_duration = 4.0f;
  float drive_speed = 40.0f;

  float snake_timer = 0.0f;
  bool disintegrating = false;
  float disintegrate_t = 0.0f;
  const float drive_turn_rate = 0.0f; // heads straight; the body squiggles
  float chase_back = 50.0f;           // camera distance behind the metro
  float chase_height = 70.0f;         // camera height above the metro

  // City names rain down during the snake phase; fall speed tracks waveOmega.
  const char *city_names[] = {"Kivenlahti",
                              "Espoonlahti",
                              "Soukka",
                              "Kaitaa",
                              "Finnoo",
                              "Matinkylä",
                              "Niittykumpu",
                              "Urheilupuisto",
                              "Tapiola",
                              "Aalto-yliopisto",
                              "Keilaniemi",
                              "Koivusaari",
                              "Lauttasaari",
                              "Ruoholahti",
                              "Kamppi",
                              "Rautatientori",
                              "Helsingin yliopisto",
                              "Hakaniemi",
                              "Sörnäinen",
                              "Kalasatama",
                              "Kulosaari",
                              "Herttoniemi",
                              "Siilitie",
                              "Itäkeskus",
                              "Myllypuro",
                              "Kontula",
                              "Mellunmäki",
                              "Puotila",
                              "Rastila",
                              "Vuosaari",
                              "Rooma",
                              "Kairo",
                              "New Delhi",
                              "Tokio",
                              "Kapkaupunki",
                              "Buenos Aires",
                              "Sydney",
                              "Auckland",
                              "Dunedin"};
  const int city_count = (int)(sizeof(city_names) / sizeof(city_names[0]));
  const float city_fall_factor = 55.0f; // pixels/sec per unit of waveOmega
  const float city_spawn_interval = 1.1f;
  std::vector<FallingCity> fallingCities;
  int next_city = 0;
  float city_spawn_timer = 0.0f;

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float current_time = (float)GetTime();
    if (current_time < 5.0f) {
      UpdateCamera(&camera, CAMERA_ORBITAL);
    }
    if (!zoom_done && current_time >= 5.0f) {
      Vector3 door_location = GetMetroDoorLocation(&primary, 2, true);
      float distance = Vector3Distance(door_location, camera.position);
      float target_distance = Vector3Distance(camera.target, door_location);
      if (!zoom_path_started) {
        zoom_path_started = true;
        zoom_start = camera.position;
        zoom_end = door_location;
        zoom_t = 0.0f;
      }
      zoom_t += GetFrameTime() / zoom_duration;
      if (zoom_t > 1.0f)
        zoom_t = 1.0f;
      camera.position = FloorParallelArcPoint(
          zoom_start, zoom_end, EaseInOut(zoom_t), zoom_arc_height);
      if (zoom_t >= 1.0f || distance < 0.08f) {
        zoom_done = true;
      }
      if (target_distance >= 0.01f) {
        float turn_alpha = 1.0f - expf(-turn_responsiveness * GetFrameTime());
        camera.target = Vector3Lerp(camera.target, door_location, turn_alpha);
      }
    }

    if (zoom_done && current_time >= 5.833f) {
      Vector3 center = GetMetroInsideLocation(&primary, 2);
      Vector3 target_location = GetMetroEndLocation(&primary);
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
        if (ride_t > 1.0f)
          ride_t = 1.0f;
        camera.position = Vector3Lerp(ride_start, ride_end, EaseInOut(ride_t));
      }
      if (ride_t >= 1.0f || distance < 0.3f) {
        ride_done = true;
      }
      if (target_distance >= 0.001f) {
        float turn_alpha = 1.0f - expf(-turn_responsiveness * GetFrameTime());
        camera.target = Vector3Lerp(camera.target, target_location, turn_alpha);
      }
    }

    // Once stopped inside the metro, fly down its length at an increasing
    // speed.
    if (ride_done && fly_progress < 1.0f) {
      if (!fly_started) {
        fly_started = true;
        fly_speed = fly_initial_speed;
        fly_progress = 0.0f;
        // Spawn additional carriages at the tail so there is a long tunnel to
        // accelerate through. Existing cars keep their positions.
        primary.numCars = 10;
        RebuildMetro(&primary);
      }
      fly_speed += fly_accel * GetFrameTime();
      fly_progress += fly_speed * GetFrameTime();
      if (fly_progress > 1.0f)
        fly_progress = 1.0f;

      camera.position = GetMetroPathPoint(&primary, fly_progress);

      float look_ahead = fly_progress + 0.1f;
      if (look_ahead > 1.0f)
        look_ahead = 1.0f;
      Vector3 ahead = GetMetroPathPoint(&primary, look_ahead);
      float turn_alpha = 1.0f - expf(-turn_responsiveness * GetFrameTime());
      camera.target = Vector3Lerp(camera.target, ahead, turn_alpha);
    }

    // Pre-build incoming star metros during late fly-through so spin start
    // does not hitch on 5x geometry generation in a single frame.
    if (fly_started && fly_progress >= star_prewarm_at && !star_prewarmed) {
      star_prewarmed = true;
      Vector3 star_center = primary.center;
      InitStarMetroTemplate(star_center, 6, metro_size);
      const Metro &tpl = GetStarMetroTemplate();
      pendingStarMetros.reserve(star_extra);
      for (int k = 0; k < star_extra; k++) {
        Metro m = CloneMetro(tpl, star_center, (Vector3){0, 0, 0});
        float a = (float)k * (2.0f * PI / (float)star_extra);
        Vector3 dir = {cosf(a), 0.0f, sinf(a)};
        m.position = Vector3Scale(dir, star_radius);
        m.yaw = -a;
        m.velocity = Vector3Scale(dir, -star_speed);
        pendingStarMetros.push_back(m);
      }
    }

    // Fly-through complete: spin the camera a full 360 in place (before it
    // rises) while the original train rides out of frame and the incoming
    // trains appear. Keeping the original framed is no longer needed.
    if (fly_started && fly_progress >= 1.0f && spin_t < 1.0f) {
      if (!spin_started) {
        spin_started = true;
        spin_pivot = camera.position;
        Vector3 fwd = Vector3Subtract(camera.target, camera.position);
        spin_start_angle = atan2f(fwd.z, fwd.x);

        // Send the original train riding out along its length.
        primary.velocity = (Vector3){ride_out_speed, 0.0f, 0.0f};

        // Activate pre-built star metros (geometry was created during
        // fly-through).
        metros = std::move(pendingStarMetros);
        star_started = true;
      }

      spin_t += GetFrameTime() / spin_duration;
      if (spin_t > 1.0f)
        spin_t = 1.0f;
      float ang = spin_start_angle + spin_t * (2.0f * PI);
      camera.position = spin_pivot;
      camera.target = (Vector3){spin_pivot.x + cosf(ang) * spin_look_dist,
                                spin_pivot.y - 3.0f,
                                spin_pivot.z + sinf(ang) * spin_look_dist};
    }

    // After the spin, rise to the overhead vantage and aim at the star center
    // where the incoming trains meet.
    if (spin_t >= 1.0f && return_t < 1.0f) {
      if (!return_path_started) {
        return_path_started = true;
        return_start = camera.position;
        return_focus_start = camera.target;
        return_t = 0.0f;
      }
      return_t += GetFrameTime() / return_duration;
      if (return_t > 1.0f)
        return_t = 1.0f;
      float eased_return_t = EaseInOut(return_t);
      Vector3 star_view_near =
          (Vector3){primary.center.x, primary.center.y + 130.0f,
                    primary.center.z + 80.0f};
      camera.position =
          Vector3Lerp(return_start, star_view_near, eased_return_t);
      camera.target =
          Vector3Lerp(return_focus_start, primary.center, eased_return_t);
    }

    // Move the trains independently of the camera: the original keeps riding
    // out, the extras converge toward the center and stop there.
    if (spin_started) {
      float dt = GetFrameTime();
      UpdateMetro(&primary, dt);
      for (Metro &m : metros) {
        UpdateMetro(&m, dt);
        // Stop once the metro reaches/overshoots the center point.
        if (Vector3DotProduct(m.position, m.velocity) > 0.0f) {
          m.position = (Vector3){0, 0, 0};
          m.velocity = (Vector3){0, 0, 0};
        }
      }
    }

    // Once the flower is fully formed, snapshot it into a texture and scatter
    // many cheap copies on the ground far outside the current view.
    if (!flowers_spawned) {
      bool flower_formed = star_started && return_t >= 1.0f && !metros.empty();
      for (const Metro &m : metros) {
        if (m.velocity.x != 0.0f || m.velocity.y != 0.0f ||
            m.velocity.z != 0.0f) {
          flower_formed = false;
        }
      }
      if (flower_formed) {
        flowers_spawned = true;
        bloom_zoom_start = camera.position;

        // Top-down orthographic snapshot of the real flower (transparent bg).
        Camera3D snapCam = {0};
        snapCam.position = (Vector3){
            primary.center.x, primary.center.y + 100.0f, primary.center.z};
        snapCam.target = primary.center;
        snapCam.up = (Vector3){0.0f, 0.0f, -1.0f};
        snapCam.fovy = flower_size; // orthographic height in world units
        snapCam.projection = CAMERA_ORTHOGRAPHIC;

        BeginTextureMode(flowerRT);
        ClearBackground(BLANK);
        BeginMode3D(snapCam);
        for (const Metro &m : metros) {
          DrawMetro(&m);
        }
        EndMode3D();
        EndTextureMode();
        flowerPlane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            flowerRT.texture;

        // Lay out a grid of cheap flowers around the real one (skip center).
        for (int gx = -flower_grid; gx <= flower_grid; gx++) {
          for (int gz = -flower_grid; gz <= flower_grid; gz++) {
            if (gx == 0 && gz == 0)
              continue;
            flowerPositions.push_back((Vector3){
                primary.center.x + gx * flower_spacing, primary.center.y,
                primary.center.z + gz * flower_spacing});
          }
        }
      }
    }

    // Zoom out to reveal the field of cheap flowers.
    if (flowers_spawned && bloom_zoom_t < 1.0f) {
      bloom_zoom_t += GetFrameTime() / bloom_zoom_duration;
      if (bloom_zoom_t > 1.0f)
        bloom_zoom_t = 1.0f;
      float extent = (float)flower_grid * flower_spacing;
      Vector3 far_view =
          (Vector3){primary.center.x, primary.center.y + extent * 0.65f,
                    primary.center.z + extent * 0.4f};
      camera.position =
          Vector3Lerp(bloom_zoom_start, far_view, EaseInOut(bloom_zoom_t));
      camera.target = primary.center;
    }

    // After the zoom out, zoom back in toward the chosen flower metro, ending
    // exactly at the follow pose so attaching is seamless.
    if (flowers_spawned && bloom_zoom_t >= 1.0f && !attached &&
        !metros.empty()) {
      if (!zoomin_started) {
        zoomin_started = true;
        zoomin_t = 0.0f;
        zoomin_start = camera.position;
        zoomin_target_start = camera.target;
        Metro &c = metros[chosen_idx];
        Vector3 cPos = Vector3Add(c.center, c.position);
        Vector3 fwd = (Vector3){cosf(c.yaw), 0.0f, -sinf(c.yaw)};
        follow_pose =
            Vector3Add(Vector3Subtract(cPos, Vector3Scale(fwd, chase_back)),
                       (Vector3){0.0f, chase_height, 0.0f});
        follow_target = cPos;
      }
      zoomin_t += GetFrameTime() / zoomin_duration;
      if (zoomin_t > 1.0f)
        zoomin_t = 1.0f;
      float ez = EaseInOut(zoomin_t);
      camera.position = Vector3Lerp(zoomin_start, follow_pose, ez);
      camera.target = Vector3Lerp(zoomin_target_start, follow_target, ez);

      // The cheap flowers are off screen by now; drop them.
      if (!flowers_cleared && zoomin_t > 0.6f) {
        flowerPositions.clear();
        flowers_cleared = true;
      }

      if (zoomin_t >= 1.0f) {
        attached = true;
        chosenMetro = metros[chosen_idx];
        metros.erase(metros.begin() + chosen_idx);
        drive_heading = chosenMetro.yaw;
        // Turn the rigid body into a slithering snake.
        chosenMetro.waveAmp = 8.0f;
        chosenMetro.waveK = 2.0f * PI / 30.0f;
        chosenMetro.waveOmega = 3.0f;
        city_spawn_timer =
            city_spawn_interval; // first city appears immediately
      }
    }

    // Drive the chosen metro away while it squiggles. Remaining flower metros
    // are culled once they leave the screen.
    if (attached) {
      float dt = GetFrameTime();
      snake_timer += dt;

      // Zoom in between 0 and 3 seconds
      if (snake_timer <= 3.0f) {
        float zoom_alpha = snake_timer / 3.0f;
        // Smoothly interpolate chase parameters
        chase_back = 50.0f - (35.0f * zoom_alpha);   // down to 15
        chase_height = 70.0f - (60.0f * zoom_alpha); // down to 10
      } else if (snake_timer > 3.0f && snake_timer <= 10.0f) {
        chase_back = 15.0f;
        chase_height = 10.0f;
      }

      if (snake_timer > 10.0f) {
        disintegrating = true;
      }

      if (disintegrating) {
        disintegrate_t += dt / 3.0f;
        if (disintegrate_t > 3.0f)
          break; // exit demoscene after 6 seconds of extra particles

        float t_clamp = fminf(disintegrate_t, 1.0f);
        // Decelerate the train
        drive_speed = 40.0f - (35.0f * t_clamp); // down to 5

        // Smoothly drop wave amplitude
        chosenMetro.waveAmp = 8.0f * (1.0f - t_clamp);
      }

      drive_heading += drive_turn_rate * dt;
      chosenMetro.yaw = drive_heading;
      Vector3 fwd =
          (Vector3){cosf(chosenMetro.yaw), 0.0f, -sinf(chosenMetro.yaw)};
      fwd = Vector3Scale(fwd, -1.0f);
      chosenMetro.position =
          Vector3Add(chosenMetro.position, Vector3Scale(fwd, drive_speed * dt));
      chosenMetro.waveOmega *= 1.001f; // squiggle accelerates over time
      chosenMetro.waveTime += dt;
      chosenMetro.disintegrateAmount = disintegrate_t;

      // Spawn city labels at the top; fall speed matches squiggle phase speed.
      float fall_speed = chosenMetro.waveOmega * city_fall_factor;
      city_spawn_timer += dt;
      if (city_spawn_timer >= city_spawn_interval) {
        city_spawn_timer = 0.0f;
        const char *name = city_names[next_city];
        next_city = (next_city + 1) % city_count;
        int fontSize = 48;
        float textW = (float)MeasureText(name, fontSize);
        float sw = (float)virtualWidth;
        fallingCities.push_back(
            {name,
             sw * 0.1f + (sw * 0.8f - textW) *
                             ((float)GetRandomValue(0, 1000) / 1000.0f),
             -60.0f, fontSize});
      }
      for (size_t i = 0; i < fallingCities.size();) {
        fallingCities[i].y += fall_speed * dt;
        if (fallingCities[i].y > (float)virtualHeight + 80.0f) {
          fallingCities.erase(fallingCities.begin() + i);
        } else {
          i++;
        }
      }

      // Follow the snake from a high behind-and-above chase angle.
      Vector3 cPos = Vector3Add(chosenMetro.center, chosenMetro.position);
      Vector3 camGoal =
          Vector3Add(Vector3Subtract(cPos, Vector3Scale(fwd, chase_back)),
                     (Vector3){0.0f, chase_height, 0.0f});
      float follow_alpha = 1.0f - expf(-3.0f * dt);
      camera.position = Vector3Lerp(camera.position, camGoal, follow_alpha);
      camera.target = cPos;

      int sw = virtualWidth;
      int sh = virtualHeight;
      Vector3 camFwd =
          Vector3Normalize(Vector3Subtract(camera.target, camera.position));
      for (size_t i = 0; i < metros.size();) {
        Vector3 p = Vector3Add(metros[i].center, metros[i].position);
        bool behind = Vector3DotProduct(Vector3Subtract(p, camera.position),
                                        camFwd) <= 0.0f;
        Vector2 s = GetWorldToScreen(p, camera);
        bool offscreen = behind || s.x < -200.0f || s.x > sw + 200.0f ||
                         s.y < -200.0f || s.y > sh + 200.0f;
        if (offscreen) {
          metros.erase(metros.begin() + i);
        } else {
          i++;
        }
      }
    }

    BeginTextureMode(target);
    ClearBackground(BLACK);

    BeginMode3D(camera);
    if (disintegrate_t <= 1.0f) {
      DrawMetro(&primary);
      for (const Metro &m : metros) {
        DrawMetro(&m);
      }
    }
    if (attached) {
      DrawMetro(&chosenMetro);
    }
    if (disintegrate_t <= 1.0f) {
      // Cheap textured-quad flowers: one draw call each.
      for (const Vector3 &p : flowerPositions) {
        DrawModel(flowerPlane, p, 1.0f, WHITE);
      }
    }
    EndMode3D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    BeginShaderMode(bloomShader);

    float scale = fmin((float)GetRenderWidth() / virtualWidth,
                       (float)GetRenderHeight() / virtualHeight);
    Rectangle sourceRec = {0.0f, 0.0f, (float)target.texture.width,
                           (float)-target.texture.height};
    Rectangle destRec = {
        (GetRenderWidth() - ((float)virtualWidth * scale)) * 0.5f,
        (GetRenderHeight() - ((float)virtualHeight * scale)) * 0.5f,
        (float)virtualWidth * scale, (float)virtualHeight * scale};

    DrawTexturePro(target.texture, sourceRec, destRec, (Vector2){0, 0}, 0.0f,
                   WHITE);
    EndShaderMode();

    if (current_time < 5.0f) {
      int fontSize = (int)((150 + 50 * sinf(current_time * 6.0f)) * scale);
      DrawText("METRO", destRec.x + 400 * scale, destRec.y + 400 * scale,
               fontSize, LIGHTGRAY);

      int fontSizeSmall = (int)((40 + 10 * sinf(current_time * 6.0f)) * scale);
      DrawText("Graffathon 26", destRec.x + 400 * scale,
               destRec.y + 550 * scale, fontSizeSmall, DARKGRAY);
    }

    if (attached && disintegrate_t <= 1.0f) {
      for (const FallingCity &c : fallingCities) {
        DrawText(c.name, destRec.x + c.x * scale, destRec.y + c.y * scale,
                 (int)(c.fontSize * scale), LIGHTGRAY);
      }
    }

    if (disintegrate_t > 1.0f) {
      const char *credit1 = "metro metro metro";
      const char *credit2 = "milkkis roosuli ropsukka";
      const char *credit3 = "Thanks for watching!";
      int fontSize = (int)(80 * scale);
      int w1 = MeasureText(credit1, fontSize);
      int w2 = MeasureText(credit2, fontSize);
      int w3 = MeasureText(credit3, fontSize);
      DrawText(credit1, destRec.x + (virtualWidth * scale - w1) * 0.5f,
               destRec.y + 200 * scale, fontSize, LIGHTGRAY);
      DrawText(credit2, destRec.x + (virtualWidth * scale - w2) * 0.5f,
               destRec.y + 350 * scale, fontSize, LIGHTGRAY);
      DrawText(credit3, destRec.x + (virtualWidth * scale - w3) * 0.5f,
               destRec.y + 500 * scale, fontSize, LIGHTGRAY);
    }

    EndDrawing();
  }

  UnloadMetroResources();
  UnloadShader(bloomShader);
  UnloadModel(flowerPlane);
  UnloadRenderTexture(flowerRT);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
