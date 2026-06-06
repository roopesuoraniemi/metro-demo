#pragma once
#include "raylib.h"

void InitMetro();
void DrawFinnishMetro();
void UnloadMetro();
Vector3 GetMetroDoorLocation(int carIndex, bool side);