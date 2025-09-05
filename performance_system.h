#ifndef PERFORMANCE_SYSTEM_H
#define PERFORMANCE_SYSTEM_H

#include "raylib.h"
#include <float.h>  // For FLT_MAX
#include <vector>
#include <chrono>
#include <string>
#include <memory>   // For smart pointers if needed

/// \brief Timer for individual systems.
/// Tracks timing for specific systems with average and max times.
struct SystemTimer {
    std::chrono::high_resolution_clock::time_point start_time_;
    float total_time_ = 0.0f;
    float max_time_ = 0.0f;
    int call_count_ = 0;
    std::string name_;
    
    /// \brief Constructor.
    /// \param timerName Name of the timer.
    SystemTimer(const std::string& timerName);

    /// \brief Starts the timer.
    void start();

    /// \brief Ends the timer and updates stats.
    void end();

    /// \brief Resets the timer stats.
    void reset();

    /// \brief Gets average time in ms.
    /// \return Average time.
    float getAverageMs() const;

    /// \brief Gets max time in ms.
    /// \return Max time.
    float getMaxMs() const;
};

/// \brief Advanced frame statistics.
struct AdvancedFrameStats {
    // Basic frame timing
    float average_frame_time_ = 0.0f;
    float min_frame_time_ = FLT_MAX;
    float max_frame_time_ = 0.0f;
    int frame_count_ = 0;
    
    // Performance budget tracking (60fps = 16.67ms target)
    static constexpr float TARGET_FRAME_TIME = 1.0f / 60.0f;  // 16.67ms
    static constexpr float WARNING_THRESHOLD = TARGET_FRAME_TIME * 1.2f;  // 20ms
    static constexpr float CRITICAL_THRESHOLD = TARGET_FRAME_TIME * 1.5f;  // 25ms
    
    int budget_warnings_ = 0;
    int budget_critical_ = 0;
    float worst_frame_time_ = 0.0f;
    
    // Rolling frame history for smooth averaging
    std::vector<float> frame_history_;
    static constexpr int HISTORY_SIZE = 60;  // 1 second at 60fps
    int history_index_ = 0;
    
    // System-specific performance tracking
    SystemTimer collision_timer_{"Collision"};
    SystemTimer rendering_timer_{"Rendering"};
    SystemTimer input_timer_{"Input"};
    SystemTimer ui_timer_{"UI"};
    SystemTimer physics_timer_{"Physics"};
    
    // Performance state
    bool show_detailed_stats_ = false;
    bool performance_warning_active_ = false;
    
    /// \brief Constructor.
    AdvancedFrameStats();
};

/// \brief Main performance monitoring system.
/// Handles updating and rendering performance stats.
class PerformanceMonitorSystem {
public:
    /// \brief Constructor.
    PerformanceMonitorSystem();

    /// \brief Updates stats with delta time.
    /// \param deltaTime Time since last frame.
    void update(float deltaTime);

    /// \brief Renders overlay.
    /// \param x X position.
    /// \param y Y position.
    void renderOverlay(int x = 10, int y = 320);

    /// \brief Resets all stats.
    void reset();

    /// \brief Checks if performance is good.
    /// \return True if good.
    bool isGood() const;

    /// \brief Gets performance report string.
    /// \return Report string.
    std::string getReport() const;

    /// \brief Logs any warnings.
    void logWarnings() const;

    // Accessors
    /// \brief Gets stats reference.
    /// \return Stats.
    const AdvancedFrameStats& getStats() const;

    /// \brief Toggles detailed stats display.
    void toggleDetailedStats();

private:
    AdvancedFrameStats stats_;
};

/// \brief Scoped timer for RAII timing.
class ScopedTimer {
public:
    /// \brief Constructor starts timer.
    /// \param timer Timer to use.
    ScopedTimer(SystemTimer& timer);

    /// \brief Destructor ends timer.
    ~ScopedTimer();
private:
    SystemTimer& timer_;
};

// Macro to use scoped timer
#define PROFILE_SYSTEM(perfSystem, system) ScopedTimer _timer(perfSystem.getStats().system##_timer_)

// Legacy compatibility
struct FrameStats {
    float average_frame_time_ = 0.0f;
    float min_frame_time_ = FLT_MAX;
    float max_frame_time_ = 0.0f;
    int frame_count_ = 0;
};

/// \brief Updates legacy frame stats.
/// \param stats Stats to update.
/// \param deltaTime Delta time.
void updateFrameStats(FrameStats& stats, float deltaTime);

/// \brief Renders legacy overlay.
/// \param stats Stats to render.
void renderPerformanceOverlay(const FrameStats& stats);

#endif