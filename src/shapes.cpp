#include "shapes.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>

static std::vector<Matrix> orangeTransforms;
static std::vector<Matrix> grayTransforms;
static std::vector<Matrix> lightGrayTransforms;
static std::vector<Matrix> blueTransforms;

static Model dotModel;
static Shader instancingShader;

void InitMetro() {
  // The size of the cube is dotRadius * 2.0f
  // The distance between the center of each cube is step.
  // The physical gap is (step - (dotRadius * 2.0f))
  float step = 0.08f;
  float dotRadius = 0.025f;

  // Create the base cube model
  Mesh cubeMesh =
      GenMeshCube(dotRadius * 2.0f, dotRadius * 2.0f, dotRadius * 2.0f);
  dotModel = LoadModelFromMesh(cubeMesh);

  // Load custom instancing shaders
  instancingShader = LoadShader("instancing.vs", "instancing.fs");

  // Set shader locations for instancing
  instancingShader.locs[SHADER_LOC_MATRIX_MVP] =
      GetShaderLocation(instancingShader, "mvp");
  instancingShader.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocationAttrib(instancingShader, "instanceTransform");
  instancingShader.locs[SHADER_LOC_COLOR_DIFFUSE] =
      GetShaderLocation(instancingShader, "colDiffuse");

  // Assign the custom shader to the model material
  dotModel.materials[0].shader = instancingShader;

  Vector3 center = {50.0f, 2.0f, -25.0f};
  int numCars = 4;
  float carLength = 10.0f;
  float carWidth = 2.5f;
  float carHeight = 2.5f;
  float gap = 0.25f;

  auto getColorGroup = [&](Vector3 p,
                           Vector3 carCenter) -> std::vector<Matrix> & {
    float minY = carCenter.y - carHeight / 2.0f;
    float maxY = carCenter.y + carHeight / 2.0f;
    float minZ = carCenter.z - carWidth / 2.0f;
    float maxZ = carCenter.z + carWidth / 2.0f;

    // Roof is light gray
    if (p.y >= maxY - step / 2.0f)
      return lightGrayTransforms;

    // Sides check for windows and doors
    if (p.z <= minZ + step / 2.0f || p.z >= maxZ - step / 2.0f) {
      float relX = p.x - carCenter.x;
      float relY = p.y - carCenter.y;

      // Doors (1 in the middle)
      float doorOffset = 0.0f;
      if (relX >= doorOffset - 0.5f && relX <= doorOffset + 0.5f &&
          relY >= -1.1f && relY <= 0.7f) {
        return grayTransforms;
      }

      // Windows (2 on each side)
      float windowOffsets[4] = {-3.5f, -1.8f, 1.8f, 3.5f};
      for (int w = 0; w < 4; w++) {
        if (relX >= windowOffsets[w] - 0.6f &&
            relX <= windowOffsets[w] + 0.6f && relY >= -0.1f && relY <= 0.7f) {
          return blueTransforms;
        }
      }
    }

    // Main body is orange
    return orangeTransforms;
  };

  auto addDot = [&](Vector3 pos, Vector3 carCenter) {
    Matrix transform = MatrixTranslate(pos.x, pos.y, pos.z);
    getColorGroup(pos, carCenter).push_back(transform);
  };

  for (int i = 0; i < numCars; i++) {
    Vector3 carCenter = {center.x +
                             (i - (numCars - 1) / 2.0f) * (carLength + gap),
                         center.y, center.z};

    float minX = carCenter.x - carLength / 2.0f;
    float maxX = carCenter.x + carLength / 2.0f;
    float minY = carCenter.y - carHeight / 2.0f;
    float maxY = carCenter.y + carHeight / 2.0f;
    float minZ = carCenter.z - carWidth / 2.0f;
    float maxZ = carCenter.z + carWidth / 2.0f;

    // Top and Bottom faces
    for (float x = minX; x <= maxX + 0.001f; x += step) {
      for (float z = minZ; z <= maxZ + 0.001f; z += step) {
        addDot((Vector3){x, maxY, z}, carCenter);
        addDot((Vector3){x, minY, z}, carCenter);
      }
    }

    // Front and Back faces
    for (float x = minX; x <= maxX + 0.001f; x += step) {
      for (float y = minY + step; y <= maxY - step + 0.001f; y += step) {
        addDot((Vector3){x, y, maxZ}, carCenter);
        addDot((Vector3){x, y, minZ}, carCenter);
      }
    }

    // Left and Right faces
    for (float y = minY + step; y <= maxY - step + 0.001f; y += step) {
      for (float z = minZ + step; z <= maxZ - step + 0.001f; z += step) {
        addDot((Vector3){minX, y, z}, carCenter);
        addDot((Vector3){maxX, y, z}, carCenter);
      }
    }
  }
}

void DrawFinnishMetro() {
  // Draw Orange
  if (!orangeTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
        Color{255, 50, 0, 255};
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      orangeTransforms.data(), orangeTransforms.size());
  }

  // Draw Gray
  if (!grayTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      grayTransforms.data(), grayTransforms.size());
  }

  // Draw Light Gray
  if (!lightGrayTransforms.empty()) {
    dotModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = LIGHTGRAY;
    DrawMeshInstanced(dotModel.meshes[0], dotModel.materials[0],
                      lightGrayTransforms.data(), lightGrayTransforms.size());
  }

  // Draw Blue
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

// Returns the center position of a door on the specified metro car.
Vector3 GetMetroDoorLocation(int carIndex, bool side) {
  Vector3 trainCenter = {50.0f, 2.0f, -25.0f};
  int numCars = 4;
  float carLength = 10.0f;
  float carWidth = 2.5f;
  float gap = 0.5f;

  if (carIndex < 0)
    carIndex = 0;
  if (carIndex > numCars - 1)
    carIndex = numCars - 1;

  float doorX =
      trainCenter.x + (carIndex - (numCars - 1) / 2.0f) * (carLength + gap);
  ;

  // Calculate Door Y
  float doorY = trainCenter.y - 0.2f;

  // Calculate Door Z
  float doorZ = trainCenter.z + (side ? (carWidth / 2.0f) : -(carWidth / 2.0f));

  return (Vector3){doorX, doorY, doorZ};
}