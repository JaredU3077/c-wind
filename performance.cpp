#include "performance.h"

void updateFrameStats(FrameStats& stats, float deltaTime) {
    stats.frameCount++;
    stats.averageFrameTime = ((stats.averageFrameTime * (stats.frameCount - 1)) + deltaTime) / stats.frameCount;
    if (deltaTime < stats.minFrameTime) stats.minFrameTime = deltaTime;
    if (deltaTime > stats.maxFrameTime) stats.maxFrameTime = deltaTime;
}

void renderPerformanceOverlay(const FrameStats& stats) {
    DrawText(TextFormat("Avg: %.2fms | Min: %.2fms | Max: %.2fms | Frames: %d",
                       stats.averageFrameTime * 1000, stats.minFrameTime * 1000,
                       stats.maxFrameTime * 1000, stats.frameCount),
             10, 10, 16, LIME);
}