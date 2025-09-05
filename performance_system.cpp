#include "performance_system.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>  // For error handling

SystemTimer::SystemTimer(const std::string& timerName) : name_(timerName) {}

void SystemTimer::start() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

void SystemTimer::end() {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - start_time_);
    float frameTimeMs = static_cast<float>(duration.count()) / 1000.0f;  // Convert to milliseconds
    
    if (frameTimeMs < 0.0f) {
        throw std::runtime_error("Negative time measured in " + name_ + " timer");
    }
    
    total_time_ += frameTimeMs;
    max_time_ = std::max(max_time_, frameTimeMs);
    call_count_++;
}

void SystemTimer::reset() {
    total_time_ = 0.0f;
    max_time_ = 0.0f;
    call_count_ = 0;
}

float SystemTimer::getAverageMs() const {
    return call_count_ > 0 ? total_time_ / static_cast<float>(call_count_) : 0.0f;
}

float SystemTimer::getMaxMs() const {
    return max_time_;
}

AdvancedFrameStats::AdvancedFrameStats() {
    frame_history_.resize(HISTORY_SIZE, 0.0f);
}

PerformanceMonitorSystem::PerformanceMonitorSystem() = default;

void PerformanceMonitorSystem::update(float deltaTime) {
    if (deltaTime < 0.0f) {
        throw std::invalid_argument("Delta time cannot be negative");
    }

    stats_.frame_count_++;
    
    // Rolling average
    stats_.frame_history_[stats_.history_index_] = deltaTime;
    stats_.history_index_ = (stats_.history_index_ + 1) % AdvancedFrameStats::HISTORY_SIZE;
    
    float historySum = 0.0f;
    int validFrames = std::min(stats_.frame_count_, AdvancedFrameStats::HISTORY_SIZE);
    for (int i = 0; i < validFrames; i++) {
        historySum += stats_.frame_history_[i];
    }
    stats_.average_frame_time_ = historySum / static_cast<float>(validFrames);
    
    // Min/max tracking
    if (deltaTime < stats_.min_frame_time_) stats_.min_frame_time_ = deltaTime;
    if (deltaTime > stats_.max_frame_time_) {
        stats_.max_frame_time_ = deltaTime;
        stats_.worst_frame_time_ = deltaTime;
    }
    
    // Performance budget monitoring
    if (deltaTime > AdvancedFrameStats::CRITICAL_THRESHOLD) {
        stats_.budget_critical_++;
        stats_.performance_warning_active_ = true;
        std::cout << "CRITICAL: Frame time " << (deltaTime * 1000.0f) << "ms exceeded " 
                  << (AdvancedFrameStats::CRITICAL_THRESHOLD * 1000.0f) << "ms budget!" << std::endl;
    } else if (deltaTime > AdvancedFrameStats::WARNING_THRESHOLD) {
        stats_.budget_warnings_++;
        if (stats_.budget_warnings_ % 60 == 0) {  // Log every second
            std::cout << "WARNING: Frame time " << (deltaTime * 1000.0f) << "ms exceeded " 
                      << (AdvancedFrameStats::WARNING_THRESHOLD * 1000.0f) << "ms budget" << std::endl;
        }
    }
    
    // Auto-reset warning after good performance
    if (deltaTime <= AdvancedFrameStats::TARGET_FRAME_TIME && stats_.frame_count_ % 180 == 0) {
        stats_.performance_warning_active_ = false;
    }
}

void PerformanceMonitorSystem::renderOverlay(int x, int y) {
    // **PERFORMANCE WINDOW**
    int perfWidth = 380;
    int perfHeight = stats_.show_detailed_stats_ ? 180 : 100;
    
    DrawRectangle(x, y, perfWidth, perfHeight, Fade(DARKGREEN, 0.8f));
    DrawRectangleLines(x, y, perfWidth, perfHeight, LIME);
    
    // **TITLE**
    DrawText("=== PERFORMANCE MONITOR ===", x + 10, y + 5, 14, LIME);
    
    // **BASIC STATS**
    float avgMs = stats_.average_frame_time_ * 1000.0f;
    float currentFPS = 1.0f / stats_.average_frame_time_;
    Color fpsColor = (currentFPS >= 55.0f) ? GREEN : ((currentFPS >= 45.0f) ? YELLOW : RED);
    
    DrawText(TextFormat("FPS: %.1f | Frame: %.2fms", currentFPS, avgMs), x + 10, y + 25, 12, fpsColor);
    
    // **PERFORMANCE STATUS**
    if (stats_.performance_warning_active_) {
        DrawText("STATUS: PERFORMANCE ISSUES DETECTED!", x + 10, y + 40, 12, RED);
    } else if (avgMs <= AdvancedFrameStats::TARGET_FRAME_TIME * 1000.0f) {
        DrawText("STATUS: EXCELLENT PERFORMANCE", x + 10, y + 40, 12, GREEN);
    } else {
        DrawText("STATUS: ACCEPTABLE PERFORMANCE", x + 10, y + 40, 12, YELLOW);
    }
    
    // **BUDGET TRACKING**
    DrawText(TextFormat("Budget Warnings: %d | Critical: %d", stats_.budget_warnings_, stats_.budget_critical_), 
             x + 10, y + 55, 12, (stats_.budget_critical_ > 0) ? RED : YELLOW);
    
    // **WORST FRAME**
    if (stats_.worst_frame_time_ > 0.0f) {
        DrawText(TextFormat("Worst Frame: %.2fms", stats_.worst_frame_time_ * 1000.0f), 
                 x + 10, y + 70, 12, RED);
    }
    
    // **DETAILED STATS** (if enabled)
    if (stats_.show_detailed_stats_) {
        DrawText("--- SYSTEM BREAKDOWN ---", x + 10, y + 90, 12, YELLOW);
        int detailY = y + 105;
        
        DrawText(TextFormat("Collision: %.2fms (%.1f calls)", 
                 stats_.collision_timer_.getAverageMs(), static_cast<float>(stats_.collision_timer_.call_count_)), 
                 x + 10, detailY, 10, WHITE);
        detailY += 12;
        
        DrawText(TextFormat("Rendering: %.2fms (%.1f calls)", 
                 stats_.rendering_timer_.getAverageMs(), static_cast<float>(stats_.rendering_timer_.call_count_)), 
                 x + 10, detailY, 10, WHITE);
        detailY += 12;
        
        DrawText(TextFormat("Input: %.2fms (%.1f calls)", 
                 stats_.input_timer_.getAverageMs(), static_cast<float>(stats_.input_timer_.call_count_)), 
                 x + 10, detailY, 10, WHITE);
        detailY += 12;
        
        DrawText(TextFormat("UI: %.2fms (%.1f calls)", 
                 stats_.ui_timer_.getAverageMs(), static_cast<float>(stats_.ui_timer_.call_count_)), 
                 x + 10, detailY, 10, WHITE);
        detailY += 12;
        
        DrawText(TextFormat("Physics: %.2fms (%.1f calls)", 
                 stats_.physics_timer_.getAverageMs(), static_cast<float>(stats_.physics_timer_.call_count_)), 
                 x + 10, detailY, 10, WHITE);
    }
    
    // **CONTROLS**
    DrawText("Press P to toggle detailed stats", x + 10, y + perfHeight - 15, 10, LIGHTGRAY);
}

void PerformanceMonitorSystem::reset() {
    stats_.frame_count_ = 0;
    stats_.average_frame_time_ = 0.0f;
    stats_.min_frame_time_ = FLT_MAX;
    stats_.max_frame_time_ = 0.0f;
    stats_.budget_warnings_ = 0;
    stats_.budget_critical_ = 0;
    stats_.worst_frame_time_ = 0.0f;
    stats_.performance_warning_active_ = false;
    
    // Reset system timers
    stats_.collision_timer_.reset();
    stats_.rendering_timer_.reset();
    stats_.input_timer_.reset();
    stats_.ui_timer_.reset();
    stats_.physics_timer_.reset();
    
    // Clear frame history
    std::fill(stats_.frame_history_.begin(), stats_.frame_history_.end(), 0.0f);
    stats_.history_index_ = 0;
}

bool PerformanceMonitorSystem::isGood() const {
    float avgMs = stats_.average_frame_time_ * 1000.0f;
    return avgMs <= AdvancedFrameStats::TARGET_FRAME_TIME * 1000.0f && 
           stats_.budget_critical_ == 0;
}

std::string PerformanceMonitorSystem::getReport() const {
    float avgMs = stats_.average_frame_time_ * 1000.0f;
    float targetMs = AdvancedFrameStats::TARGET_FRAME_TIME * 1000.0f;
    
    if (avgMs <= targetMs) {
        return "EXCELLENT - Meeting 60fps target";
    } else if (avgMs <= targetMs * 1.2f) {
        return "GOOD - Minor frame time variance";
    } else if (avgMs <= targetMs * 1.5f) {
        return "ACCEPTABLE - Some performance issues";
    } else {
        return "POOR - Significant performance problems";
    }
}

void PerformanceMonitorSystem::logWarnings() const {
    if (stats_.budget_critical_ > 0) {
        std::cout << "PERFORMANCE ALERT: " << stats_.budget_critical_ 
                  << " critical frame time violations detected!" << std::endl;
    }
    
    if (stats_.budget_warnings_ > 60) {  // More than 1 second of warnings
        std::cout << "PERFORMANCE WARNING: " << stats_.budget_warnings_ 
                  << " frame time budget warnings" << std::endl;
    }
}

const AdvancedFrameStats& PerformanceMonitorSystem::getStats() const {
    return stats_;
}

void PerformanceMonitorSystem::toggleDetailedStats() {
    stats_.show_detailed_stats_ = !stats_.show_detailed_stats_;
}

ScopedTimer::ScopedTimer(SystemTimer& timer) : timer_(timer) {
    timer_.start();
}

ScopedTimer::~ScopedTimer() {
    timer_.end();
}

// Legacy functions
void updateFrameStats(FrameStats& stats, float deltaTime) {
    stats.frame_count_++;
    stats.average_frame_time_ = ((stats.average_frame_time_ * (stats.frame_count_ - 1)) + deltaTime) / static_cast<float>(stats.frame_count_);
    if (deltaTime < stats.min_frame_time_) stats.min_frame_time_ = deltaTime;
    if (deltaTime > stats.max_frame_time_) stats.max_frame_time_ = deltaTime;
}

void renderPerformanceOverlay(const FrameStats& stats) {
    DrawText(TextFormat("Avg: %.2fms | Min: %.2fms | Max: %.2fms | Frames: %d",
                       stats.average_frame_time_ * 1000, stats.min_frame_time_ * 1000,
                       stats.max_frame_time_ * 1000, stats.frame_count_),
             10, 10, 16, LIME);
}