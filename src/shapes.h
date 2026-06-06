#pragma once
#include "raylib.h"
#include <vector>

typedef struct Metro {
  Vector3 center;   // curve vertex + layout anchor (world)
  Vector3 position; // dynamic world offset (starts {0,0,0})
  Vector3 velocity; // units/second
  float yaw;        // rotation about Y, applied about the metro center (radians)
  int numCars;
  int layoutCars; // anchor count so spawned cars only extend the tail
  float size;
  // Snake undulation (0 amplitude = rigid body). waveK = 2*PI/wavelength,
  // waveOmega = slither speed, waveTime accumulates while waveAmp > 0.
  float waveAmp, waveK, waveOmega, waveTime;
  std::vector<Matrix> orange, gray, lightGray, blue;
} Metro;

void InitMetroResources();
void UnloadMetroResources();

Metro CreateMetro(Vector3 center, int numCars, float size, Vector3 velocity);
Metro CloneMetro(const Metro &src, Vector3 center, Vector3 velocity);
void InitStarMetroTemplate(Vector3 center, int numCars, float size);
const Metro &GetStarMetroTemplate();
void RebuildMetro(Metro *m);
void UpdateMetro(Metro *m, float dt);
void DrawMetro(const Metro *m);

Vector3 GetMetroDoorLocation(const Metro *m, int carIndex, bool side);
Vector3 GetMetroEndLocation(const Metro *m);
Vector3 GetMetroInsideLocation(const Metro *m, int carIndex);
Vector3 GetMetroPathPoint(const Metro *m, float t);
Vector3 GetMetroCurveCenter(const Metro *m);
