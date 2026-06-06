#include "shapes.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>

#define GAP 0.025
#define STEP_SIZE 0.08
#define NUM_CARS 6

static std::vector<Matrix> orangeTransforms;
static std::vector<Matrix> grayTransforms;
static std::vector<Matrix> lightGrayTransforms;
static std::vector<Matrix> blueTransforms;

static Vector3 center = {50.0f, 2.0f, -25.0f};

static Model dotModel;
static Shader instancingShader;

static float MetroPathZ(float x, float size) {
  float curvature = 0.012f / size;
  float dx = x - center.x;
  float zOffset = curvature * dx * dx;
  return center.z + zOffset;
}

static float MetroPathSlope(float x, float size) {
  float curvature = 0.012f / size;
  float dx = x - center.x;
  return 2.0f * curvature * dx;
}

void InitMetro(float size) {
  orangeTransforms.clear();
  grayTransforms.clear();
  lightGrayTransforms.clear();
  blueTransforms.clear();

  float step = STEP_SIZE * size;
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

  dotModel.materials[0].shader = instancingShader;

  float carLength = 10.0f * size;
  float carWidth = 2.5f * size;
  float carHeight = 2.5f * size;
  float gap = GAP * size;

  auto getColorGroup = [&](Vector3 local) -> std::vector<Matrix> & {
    float minY = -carHeight / 2.0f;
    float maxY = carHeight / 2.0f;
    float minZ = -carWidth / 2.0f;
    float maxZ = carWidth / 2.0f;

    if (local.y >= maxY - step / 2.0f) return lightGrayTransforms;

    if (local.z <= minZ + step / 2.0f || local.z >= maxZ - step / 2.0f) {
      float relX = local.x;
      float relY = local.y;

      float doorOffset = 0.0f;
      if (relX >= doorOffset - 0.5f && relX <= doorOffset + 0.5f &&
          relY >= -1.1f && relY <= 0.7f) {
        return grayTransforms;
      }

      float windowOffsets[4] = {-3.5f, -1.8f, 1.8f, 3.5f};
      for (int w = 0; w < 4; w++) {
        if (relX >= windowOffsets[w] - 0.6f &&
            relX <= windowOffsets[w] + 0.6f && relY >= -0.1f && relY <= 0.7f) {
          return blueTransforms;
        }
      }
    }

    return orangeTransforms;
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

  for (int i = 0; i < NUM_CARS; i++) {
    float carCenterX =
        center.x + (i - (NUM_CARS - 1) / 2.0f) * (carLength + gap);
    float carCenterZ = MetroPathZ(carCenterX, size);
    float slope = MetroPathSlope(carCenterX, size);

    Vector3 carCenter = {carCenterX, center.y, carCenterZ};
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

void DrawFinnishMetro() {
  if (!orangeTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
        Color{255, 50, 0, 255};
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      orangeTransforms.data(), orangeTransforms.size());
  }

  if (!grayTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      grayTransforms.data(), grayTransforms.size());
  }

  if (!lightGrayTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = LIGHTGRAY;
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      lightGrayTransforms.data(), lightGrayTransforms.size());
  }

  if (!blueTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      blueTransforms.data(), blueTransforms.size());
  }
}

void UnloadMetro() {
  UnloadModel(dotModel);
  UnloadShader(instancingShader);
  orangeTransforms.clear();
  grayTransforms.clear();
  lightGrayTransforms.clear();
  blueTransforms.clear();
}

Vector3 GetMetroDoorLocation(int carIndex, bool side, float size) {
  float carLength = 10.0f * size;
  float carWidth = 2.5f * size;
  float gap = GAP * size;

  if (carIndex < 0) carIndex = 0;
  if (carIndex > NUM_CARS - 1) carIndex = NUM_CARS - 1;

  float carCenterX =
      center.x + (carIndex - (NUM_CARS - 1) / 2.0f) * (carLength + gap);
  float carCenterZ = MetroPathZ(carCenterX, size);
  float slope = MetroPathSlope(carCenterX, size);

  Vector3 carCenter = {carCenterX, center.y, carCenterZ};
  Vector3 forward = Vector3Normalize((Vector3){1.0f, 0.0f, slope});
  Vector3 right = (Vector3){-forward.z, 0.0f, forward.x};

  float doorLocalY = -0.2f;
  float doorLocalZ = side ? (carWidth / 2.0f) : -(carWidth / 2.0f);

  Vector3 door = carCenter;
  door = Vector3Add(door, (Vector3){0.0f, doorLocalY, 0.0f});
  door = Vector3Add(door, Vector3Scale(right, doorLocalZ));
  return door;
}

Vector3 GetMetroEndLocation(float size) {
  float carLength = 10.0f * size;
  float gap = GAP * size;

  float lastCarIndex = (float)(NUM_CARS - 1);
  float lastCarCenterX =
      center.x + (lastCarIndex - (NUM_CARS - 1) / 2.0f) * (carLength + gap);
  float lastCarCenterZ = MetroPathZ(lastCarCenterX, size);
  float slope = MetroPathSlope(lastCarCenterX, size);

  Vector3 lastCenter = {lastCarCenterX, center.y, lastCarCenterZ};
  Vector3 forward = Vector3Normalize((Vector3){1.0f, 0.0f, slope});

  return Vector3Add(lastCenter, Vector3Scale(forward, carLength / 2.0f));
}

Vector3 GetMetroInsideLocation(int carIndex, float size) {
  float carLength = 10.0f * size;
  float gap = GAP * size;

  if (carIndex < 0) carIndex = 0;
  if (carIndex > NUM_CARS - 1) carIndex = NUM_CARS - 1;

  float carCenterX =
      center.x + (carIndex - (NUM_CARS - 1) / 2.0f) * (carLength + gap);
  float carCenterZ = MetroPathZ(carCenterX, size);

  return (Vector3){carCenterX, center.y - 0.2f, carCenterZ};
}