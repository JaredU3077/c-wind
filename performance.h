#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "raylib.h"
#include <float.h>  // For FLT_MAX
#include <vector>
#include <chrono>
#include <string>

// **ENHANCED PERFORMANCE MONITORING SYSTEM**

// **LEGACY COMPATIBILITY** - Keep original FrameStats for existing code
struct FrameStats {
    float averageFrameTime = 0.0f;
    float minFrameTime = FLT_MAX;
    float maxFrameTime = 0.0f;
    int frameCount = 0;
};

struct SystemTimer {
    std::chrono::high_resolution_clock::time_point startTime;
    float totalTime = 0.0f;
    float maxTime = 0.0f;
    int callCount = 0;
    std::string name;
    
    SystemTimer(const std::string& timerName) : name(timerName) {}
    
    void start();
    void end();
    void reset();
    float getAverageMs() const;
    float getMaxMs() const;
};

struct AdvancedFrameStats {
    // Basic frame timing
    float averageFrameTime = 0.0f;
    float minFrameTime = FLT_MAX;
    float maxFrameTime = 0.0f;
    int frameCount = 0;
    
    // Performance budget tracking (60fps = 16.67ms target)
    static constexpr float TARGET_FRAME_TIME = 1.0f / 60.0f;  // 16.67ms
    static constexpr float WARNING_THRESHOLD = TARGET_FRAME_TIME * 1.2f;  // 20ms
    static constexpr float CRITICAL_THRESHOLD = TARGET_FRAME_TIME * 1.5f;  // 25ms
    
    int budgetWarnings = 0;
    int budgetCritical = 0;
    float worstFrameTime = 0.0f;
    
    // Rolling frame history for smooth averaging
    std::vector<float> frameHistory;
    static constexpr int HISTORY_SIZE = 60;  // 1 second at 60fps
    int historyIndex = 0;
    
    // System-specific performance tracking
    SystemTimer collisionTimer{"Collision"};
    SystemTimer renderingTimer{"Rendering"};
    SystemTimer inputTimer{"Input"};
    SystemTimer uiTimer{"UI"};
    SystemTimer physicsTimer{"Physics"};
    
    // Performance state
    bool showDetailedStats = false;
    bool performanceWarningActive = false;
    
    AdvancedFrameStats();
};

// **CORE FUNCTIONS**
void updateAdvancedFrameStats(AdvancedFrameStats& stats, float deltaTime);
void renderAdvancedPerformanceOverlay(const AdvancedFrameStats& stats, int x = 10, int y = 320);
void resetPerformanceStats(AdvancedFrameStats& stats);

// **SYSTEM TIMING HELPERS**
class ScopedTimer {
public:
    ScopedTimer(SystemTimer& timer) : timer_(timer) { timer_.start(); }
    ~ScopedTimer() { timer_.end(); }
private:
    SystemTimer& timer_;
};

// **CONVENIENCE MACROS**
#define PROFILE_SYSTEM(stats, system) ScopedTimer _timer(stats.system##Timer)

// **PERFORMANCE ANALYSIS**
bool isPerformanceGood(const AdvancedFrameStats& stats);
std::string getPerformanceReport(const AdvancedFrameStats& stats);
void logPerformanceWarnings(const AdvancedFrameStats& stats);

// **LEGACY COMPATIBILITY FUNCTIONS**
void updateFrameStats(FrameStats& stats, float deltaTime);
void renderPerformanceOverlay(const FrameStats& stats);

#endif