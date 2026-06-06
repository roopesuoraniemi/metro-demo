#pragma once
#include "raylib.h"

void InitMetro(float size);
void DrawFinnishMetro(Matrix transform, float disintegrateAmount);
void UnloadMetro();
Vector3 GetMetroDoorLocation(int carIndex, bool side, float size, Vector3 metroPosition);
Vector3 GetMetroEndLocation(float size, Vector3 metroPosition);
Vector3 GetMetroInsideLocation(int carIndex, float size, Vector3 metroPosition);
