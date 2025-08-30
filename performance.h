#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "raylib.h"
#include <float.h>  // For FLT_MAX

struct FrameStats {
    float averageFrameTime = 0.0f;
    float minFrameTime = FLT_MAX;
    float maxFrameTime = 0.0f;
    int frameCount = 0;
};

void updateFrameStats(FrameStats& stats, float deltaTime);
void renderPerformanceOverlay(const FrameStats& stats);

#endif