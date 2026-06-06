#pragma once
#include "raylib.h"

void InitMetro(float size);
void DrawFinnishMetro();
void UnloadMetro();
Vector3 GetMetroDoorLocation(int carIndex, bool side, float size);
Vector3 GetMetroEndLocation(float size);
Vector3 GetMetroInsideLocation(int carIndex, float size);
