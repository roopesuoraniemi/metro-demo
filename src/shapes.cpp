#include "shapes.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>

#define GAP 0.075
#define STEP_SIZE 0.1

static Model dotModel;
static Shader instancingShader;
static int gMetroTransformLoc = -1;
static int gMetroCenterLoc = -1;
static int gWaveAmpLoc = -1;
static int gWaveKLoc = -1;
static int gWaveOmegaLoc = -1;
static int gWaveTimeLoc = -1;

static Metro gStarMetroTemplate;
static bool gStarTemplateReady = false;

static float MetroPathZ(Vector3 center, float x, float size) {
  float curvature = 0.012f / size;
  float dx = x - center.x;
  float zOffset = curvature * dx * dx;
  return center.z + zOffset;
}

static float MetroPathSlope(Vector3 center, float x, float size) {
  float curvature = 0.012f / size;
  float dx = x - center.x;
  return 2.0f * curvature * dx;
}

// Car center X anchored to the layout car count. Independent of the current car
// count, so appending cars never shifts the existing ones.
static float CarCenterX(Vector3 center, int layoutCars, int carIndex,
                        float size) {
  float carLength = 10.0f * size;
  float gap = GAP * size;
  return center.x + (carIndex - (layoutCars - 1) / 2.0f) * (carLength + gap);
}

static void BuildMetroGeometry(Metro *m) {
  m->orange.clear();
  m->gray.clear();
  m->lightGray.clear();
  m->blue.clear();

  float size = m->size;
  float step = STEP_SIZE * size;

  float carLength = 10.0f * size;
  float carWidth = 2.5f * size;
  float carHeight = 2.5f * size;

  auto getColorGroup = [&](Vector3 local) -> std::vector<Matrix> & {
    float minY = -carHeight / 2.0f;
    float maxY = carHeight / 2.0f;
    float minZ = -carWidth / 2.0f;
    float maxZ = carWidth / 2.0f;

    if (local.y >= maxY - step / 2.0f) return m->lightGray;

    if (local.z <= minZ + step / 2.0f || local.z >= maxZ - step / 2.0f) {
      float relX = local.x;
      float relY = local.y;

      float doorOffset = 0.0f;
      if (relX >= doorOffset - 0.5f && relX <= doorOffset + 0.5f &&
          relY >= -1.1f && relY <= 0.7f) {
        return m->gray;
      }

      float windowOffsets[4] = {-3.5f, -1.8f, 1.8f, 3.5f};
      for (int w = 0; w < 4; w++) {
        if (relX >= windowOffsets[w] - 0.6f &&
            relX <= windowOffsets[w] + 0.6f && relY >= -0.1f && relY <= 0.7f) {
          return m->blue;
        }
      }
    }

    return m->orange;
  };

  auto toWorld = [](Vector3 local, Vector3 carCenter, Vector3 forward,
                    Vector3 right) {
    Vector3 p = carCenter;
    p = Vector3Add(p, Vector3Scale(forward, local.x));
    p = Vector3Add(p, (Vector3){0.0f, local.y, 0.0f});
    p = Vector3Add(p, Vector3Scale(right, local.z));
    return p;
  };

  auto addDot = [&](Vector3 local, Vector3 carCenter, Vector3 forward,
                    Vector3 right) {
    Vector3 pos = toWorld(local, carCenter, forward, right);
    Matrix transform = MatrixTranslate(pos.x, pos.y, pos.z);
    getColorGroup(local).push_back(transform);
  };

  for (int i = 0; i < m->numCars; i++) {
    float carCenterX = CarCenterX(m->center, m->layoutCars, i, size);
    float carCenterZ = MetroPathZ(m->center, carCenterX, size);
    float slope = MetroPathSlope(m->center, carCenterX, size);

    Vector3 carCenter = {carCenterX, m->center.y, carCenterZ};
    Vector3 forward = Vector3Normalize((Vector3){1.0f, 0.0f, slope});
    Vector3 right = (Vector3){-forward.z, 0.0f, forward.x};

    float minX = -carLength / 2.0f;
    float maxX = carLength / 2.0f;
    float minY = -carHeight / 2.0f;
    float maxY = carHeight / 2.0f;
    float minZ = -carWidth / 2.0f;
    float maxZ = carWidth / 2.0f;

    for (float x = minX; x <= maxX + 0.001f; x += step) {
      for (float z = minZ; z <= maxZ + 0.001f; z += step) {
        addDot((Vector3){x, maxY, z}, carCenter, forward, right);
        addDot((Vector3){x, minY, z}, carCenter, forward, right);
      }
    }

    for (float x = minX; x <= maxX + 0.001f; x += step) {
      for (float y = minY + step; y <= maxY - step + 0.001f; y += step) {
        addDot((Vector3){x, y, maxZ}, carCenter, forward, right);
        addDot((Vector3){x, y, minZ}, carCenter, forward, right);
      }
    }

    for (float y = minY + step; y <= maxY - step + 0.001f; y += step) {
      for (float z = minZ + step; z <= maxZ - step + 0.001f; z += step) {
        addDot((Vector3){minX, y, z}, carCenter, forward, right);
        addDot((Vector3){maxX, y, z}, carCenter, forward, right);
      }
    }
  }
}

void InitMetroResources() {
  float dotRadius = 0.025f;

  Mesh cubeMesh =
      GenMeshCube(dotRadius * 2.0f, dotRadius * 2.0f, dotRadius * 2.0f);
  dotModel = LoadModelFromMesh(cubeMesh);

  instancingShader = LoadShader("instancing.vs", "instancing.fs");

  instancingShader.locs[SHADER_LOC_MATRIX_MVP] =
      GetShaderLocation(instancingShader, "mvp");
  instancingShader.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocationAttrib(instancingShader, "instanceTransform");
  instancingShader.locs[SHADER_LOC_COLOR_DIFFUSE] =
      GetShaderLocation(instancingShader, "colDiffuse");

  gMetroTransformLoc = GetShaderLocation(instancingShader, "metroTransform");
  gMetroCenterLoc = GetShaderLocation(instancingShader, "metroCenter");
  gWaveAmpLoc = GetShaderLocation(instancingShader, "waveAmp");
  gWaveKLoc = GetShaderLocation(instancingShader, "waveK");
  gWaveOmegaLoc = GetShaderLocation(instancingShader, "waveOmega");
  gWaveTimeLoc = GetShaderLocation(instancingShader, "waveTime");

  dotModel.materials[0].shader = instancingShader;
}

void UnloadMetroResources() {
  UnloadModel(dotModel);
  UnloadShader(instancingShader);
}

Metro CreateMetro(Vector3 center, int numCars, float size, Vector3 velocity) {
  Metro m;
  m.center = center;
  m.position = (Vector3){0.0f, 0.0f, 0.0f};
  m.velocity = velocity;
  m.yaw = 0.0f;
  m.numCars = numCars;
  m.layoutCars = numCars;
  m.size = size;
  m.waveAmp = 0.0f;
  m.waveK = 0.0f;
  m.waveOmega = 0.0f;
  m.waveTime = 0.0f;
  BuildMetroGeometry(&m);
  return m;
}

Metro CloneMetro(const Metro &src, Vector3 center, Vector3 velocity) {
  Metro m;
  m.center = center;
  m.position = (Vector3){0.0f, 0.0f, 0.0f};
  m.velocity = velocity;
  m.yaw = 0.0f;
  m.numCars = src.numCars;
  m.layoutCars = src.layoutCars;
  m.size = src.size;
  m.waveAmp = 0.0f;
  m.waveK = 0.0f;
  m.waveOmega = 0.0f;
  m.waveTime = 0.0f;
  m.orange = src.orange;
  m.gray = src.gray;
  m.lightGray = src.lightGray;
  m.blue = src.blue;
  return m;
}

void InitStarMetroTemplate(Vector3 center, int numCars, float size) {
  if (!gStarTemplateReady) {
    gStarMetroTemplate = CreateMetro(center, numCars, size, (Vector3){0, 0, 0});
    gStarTemplateReady = true;
  }
}

const Metro &GetStarMetroTemplate() { return gStarMetroTemplate; }

void RebuildMetro(Metro *m) { BuildMetroGeometry(m); }

void UpdateMetro(Metro *m, float dt) {
  m->position = Vector3Add(m->position, Vector3Scale(m->velocity, dt));
  if (m->waveAmp > 0.0f) m->waveTime += dt;
}

static void DrawInstanceGroup(const std::vector<Matrix> &group, Color color) {
  if (group.empty()) return;
  dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;
  DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0], group.data(),
                    (int)group.size());
}

void DrawMetro(const Metro *m) {
  // Rotate the metro about its own center, then apply the world position
  // offset: world = position + center + RotateY(yaw) * (vertex - center).
  Matrix toLocal =
      MatrixTranslate(-m->center.x, -m->center.y, -m->center.z);
  Matrix rot = MatrixRotateY(m->yaw);
  Matrix back = MatrixTranslate(m->center.x + m->position.x,
                                m->center.y + m->position.y,
                                m->center.z + m->position.z);
  Matrix t = MatrixMultiply(MatrixMultiply(toLocal, rot), back);
  if (gMetroTransformLoc >= 0) {
    SetShaderValueMatrix(instancingShader, gMetroTransformLoc, t);
  }

  // Snake undulation parameters (waveAmp 0 -> rigid body).
  if (gMetroCenterLoc >= 0) {
    float center[3] = {m->center.x, m->center.y, m->center.z};
    SetShaderValue(instancingShader, gMetroCenterLoc, center, SHADER_UNIFORM_VEC3);
  }
  if (gWaveAmpLoc >= 0)
    SetShaderValue(instancingShader, gWaveAmpLoc, &m->waveAmp, SHADER_UNIFORM_FLOAT);
  if (gWaveKLoc >= 0)
    SetShaderValue(instancingShader, gWaveKLoc, &m->waveK, SHADER_UNIFORM_FLOAT);
  if (gWaveOmegaLoc >= 0)
    SetShaderValue(instancingShader, gWaveOmegaLoc, &m->waveOmega, SHADER_UNIFORM_FLOAT);
  if (gWaveTimeLoc >= 0)
    SetShaderValue(instancingShader, gWaveTimeLoc, &m->waveTime, SHADER_UNIFORM_FLOAT);

  DrawInstanceGroup(m->orange, Color{255, 50, 0, 255});
  DrawInstanceGroup(m->gray, GRAY);
  DrawInstanceGroup(m->lightGray, LIGHTGRAY);
  DrawInstanceGroup(m->blue, BLUE);
}

Vector3 GetMetroDoorLocation(const Metro *m, int carIndex, bool side) {
  float size = m->size;
  float carWidth = 2.5f * size;

  if (carIndex < 0) carIndex = 0;
  if (carIndex > m->numCars - 1) carIndex = m->numCars - 1;

  float carCenterX = CarCenterX(m->center, m->layoutCars, carIndex, size);
  float carCenterZ = MetroPathZ(m->center, carCenterX, size);
  float slope = MetroPathSlope(m->center, carCenterX, size);

  Vector3 carCenter = {carCenterX, m->center.y, carCenterZ};
  Vector3 forward = Vector3Normalize((Vector3){1.0f, 0.0f, slope});
  Vector3 right = (Vector3){-forward.z, 0.0f, forward.x};

  float doorLocalY = -0.2f;
  float doorLocalZ = side ? (carWidth / 2.0f) : -(carWidth / 2.0f);

  Vector3 door = carCenter;
  door = Vector3Add(door, (Vector3){0.0f, doorLocalY, 0.0f});
  door = Vector3Add(door, Vector3Scale(right, doorLocalZ));
  return Vector3Add(door, m->position);
}

Vector3 GetMetroEndLocation(const Metro *m) {
  float size = m->size;
  float carLength = 10.0f * size;

  float lastCarCenterX =
      CarCenterX(m->center, m->layoutCars, m->numCars - 1, size);
  float lastCarCenterZ = MetroPathZ(m->center, lastCarCenterX, size);
  float slope = MetroPathSlope(m->center, lastCarCenterX, size);

  Vector3 lastCenter = {lastCarCenterX, m->center.y, lastCarCenterZ};
  Vector3 forward = Vector3Normalize((Vector3){1.0f, 0.0f, slope});

  Vector3 end = Vector3Add(lastCenter, Vector3Scale(forward, carLength / 2.0f));
  return Vector3Add(end, m->position);
}

Vector3 GetMetroInsideLocation(const Metro *m, int carIndex) {
  float size = m->size;

  if (carIndex < 0) carIndex = 0;
  if (carIndex > m->numCars - 1) carIndex = m->numCars - 1;

  float carCenterX = CarCenterX(m->center, m->layoutCars, carIndex, size);
  float carCenterZ = MetroPathZ(m->center, carCenterX, size);

  Vector3 inside = {carCenterX, m->center.y - 0.2f, carCenterZ};
  return Vector3Add(inside, m->position);
}

// Returns a point along the metro's interior centerline. t = 0 is the center
// of the entry car (carIndex 2), t = 1 is the far end of the last car. The
// point follows the curved trajectory so the camera can fly down the tunnel.
Vector3 GetMetroPathPoint(const Metro *m, float t) {
  float size = m->size;
  float carLength = 10.0f * size;

  float startX = CarCenterX(m->center, m->layoutCars, 2, size);
  float endX =
      CarCenterX(m->center, m->layoutCars, m->numCars - 1, size) +
      carLength / 2.0f;

  float x = startX + (endX - startX) * t;
  Vector3 point = {x, m->center.y - 0.2f, MetroPathZ(m->center, x, size)};
  return Vector3Add(point, m->position);
}

Vector3 GetMetroCurveCenter(const Metro *m) {
  float size = m->size;
  float curvature = 0.012f / size;
  float radius = 1.0f / (2.0f * curvature);
  Vector3 c = {m->center.x, m->center.y + 8.0f, m->center.z + radius};
  return Vector3Add(c, m->position);
}
