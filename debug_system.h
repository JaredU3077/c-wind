#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <csignal>

// ============================================================================
// COMPREHENSIVE DEBUG SYSTEM FOR CRASH ISOLATION
// ============================================================================

// Global debug flags
extern bool DEBUG_MODE;
extern bool VERBOSE_LOGGING;
extern int DEBUG_LEVEL;

// Debug levels
enum DebugLevel {
    DEBUG_NONE = 0,
    DEBUG_BASIC = 1,
    DEBUG_DETAILED = 2,
    DEBUG_VERBOSE = 3,
    DEBUG_TRACE = 4
};

// Debug categories
enum DebugCategory {
    DEBUG_GENERAL,
    DEBUG_UI,
    DEBUG_INVENTORY,
    DEBUG_RENDERING,
    DEBUG_THEME,
    DEBUG_INPUT,
    DEBUG_MEMORY,
    DEBUG_PERFORMANCE
};

#ifdef BROWSERWIND_DEBUG
// Debug macros for easy usage
#define DEBUG_LOG(category, level, message) \
    if (DEBUG_MODE && DEBUG_LEVEL >= level) { \
        DebugSystem::log(category, level, __FILE__, __LINE__, __FUNCTION__, message); \
    }

#define DEBUG_TRACE_ENTER() \
    if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_TRACE) { \
        DebugSystem::traceEnter(__FUNCTION__); \
    }

#define DEBUG_TRACE_EXIT() \
    if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_TRACE) { \
        DebugSystem::traceExit(__FUNCTION__); \
    }

#define DEBUG_PERF_START(name) \
    if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_PERFORMANCE) { \
        DebugSystem::startPerformanceTimer(name); \
    }

#define DEBUG_PERF_END(name) \
    if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_PERFORMANCE) { \
        DebugSystem::endPerformanceTimer(name); \
    }

// Safe function wrapper that catches all exceptions
#define SAFE_CALL(func, description) \
    do { \
        DEBUG_TRACE_ENTER(); \
        try { \
            DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Executing: " + std::string(description)); \
            func; \
            DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Completed: " + std::string(description)); \
        } catch (const std::exception& e) { \
            DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "EXCEPTION in " + std::string(description) + ": " + std::string(e.what())); \
            std::cerr << "FATAL ERROR in " << description << ": " << e.what() << std::endl; \
        } catch (...) { \
            DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "UNKNOWN EXCEPTION in " + std::string(description)); \
            std::cerr << "FATAL UNKNOWN ERROR in " << description << std::endl; \
        } \
        DEBUG_TRACE_EXIT(); \
    } while(0)

// Memory validation macros
#define VALIDATE_POINTER(ptr, description) \
    if (DEBUG_MODE && !(ptr)) { \
        DEBUG_LOG(DEBUG_MEMORY, DEBUG_BASIC, "NULL POINTER: " + std::string(description)); \
        std::cerr << "CRITICAL: NULL pointer detected: " << description << std::endl; \
    }

// Performance monitoring
#define PERF_SCOPE(name) if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_PERFORMANCE) { PerformanceTimer perf_timer##__LINE__(name); }

#else
// Release mode: all debug macros compile to nothing
#define DEBUG_LOG(category, level, message) do {} while(0)
#define DEBUG_TRACE_ENTER() do {} while(0)
#define DEBUG_TRACE_EXIT() do {} while(0)
#define DEBUG_PERF_START(name) do {} while(0)
#define DEBUG_PERF_END(name) do {} while(0)
#define SAFE_CALL(func, description) func
#define VALIDATE_POINTER(ptr, description) do {} while(0)
#define PERF_SCOPE(name) do {} while(0)

#endif

// Performance statistics structure
struct PerformanceStats {
    int callCount;
    double totalTime;
    double avgTime;
    double minTime;
    double maxTime;
};

// PerformanceTimer class will be defined after DebugSystem

// Crash detection and reporting
class CrashReporter {
public:
    static void setupSignalHandlers();
    static void reportCrash(const std::string& location, const std::string& details);
    static void dumpCallStack();
    static std::vector<std::string> getCurrentStackTrace();
    static std::string getLastKnownLocation() { return last_known_location_; }
    static void setLastKnownLocation([[maybe_unused]] const std::string& location) { last_known_location_ = location; }

private:
    static void signalHandler(int signal);
    static std::string last_known_location_;
};

// Main debug system class
class DebugSystem {
public:
    static void initialize();
    static void shutdown();
    static void setDebugLevel(int level);
    static void enableCategory(DebugCategory category, bool enabled);

    static void log([[maybe_unused]] DebugCategory category, [[maybe_unused]] int level,
                   [[maybe_unused]] const char* file, [[maybe_unused]] int line,
                   [[maybe_unused]] const char* function, [[maybe_unused]] const std::string& message);

    static void traceEnter([[maybe_unused]] const char* function);
    static void traceExit([[maybe_unused]] const char* function);

    static void startPerformanceTimer(const std::string& name);
    static void endPerformanceTimer(const std::string& name);

    // Enhanced performance monitoring
    static void startFrameProfile();
    static void endFrameProfile();
    static void generatePerformanceReport();
    static void resetPerformanceStats();

    // Performance statistics access
    static double getAverageFPS() { return avg_fps_; }
    static double getAverageFrameTime() { return avg_frame_time_; }
    static double getCurrentFPS() { return current_fps_; }

    static void setLastKnownLocation([[maybe_unused]] const std::string& location);

    // Memory debugging
    static void checkMemoryUsage();
    static void validatePointer(void* ptr, const std::string& description);

    // Public accessors for private members
    static const std::vector<std::string>& getCallStack() { return call_stack_; }
    static void updateCallStack(size_t index, const std::string& symbol);
    static void clearCallStack();

private:
    static std::vector<std::string> call_stack_;
    static std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> perf_timers_;

    // Performance monitoring data
    static std::chrono::high_resolution_clock::time_point frame_start_;
    static std::vector<double> frame_times_;
    static std::unordered_map<std::string, PerformanceStats> perf_stats_;
    static long long frame_count_;
    static double avg_frame_time_;
    static double current_fps_;
    static double avg_fps_;
};

// Utility functions for crash isolation
namespace DebugUtils {
    void enableCoreDump();
    void setupExceptionHandling();
    std::string demangleSymbol(const char* mangled);
    void printSystemInfo();
    void validateGameState();
}

// Test framework for isolated testing
class TestFramework {
public:
    static void runInventoryTest();
    static void runUITest();
    static void runRenderingTest();
    static void runThemeTest();

    static bool testResult(const std::string& testName, bool passed,
                          const std::string& details = "");
};

// RAII performance timer - defined after DebugSystem to avoid forward declaration issues
class PerformanceTimer {
public:
    PerformanceTimer(const std::string& name) : name_(name) {
        if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_PERFORMANCE) {
            DebugSystem::startPerformanceTimer(name_);
        }
    }
    ~PerformanceTimer() {
        if (DEBUG_MODE && DEBUG_LEVEL >= DEBUG_PERFORMANCE) {
            DebugSystem::endPerformanceTimer(name_);
        }
    }
private:
    std::string name_;
};

// Initialize debug system
void initializeDebugSystem();
void setDebugMode(bool enabled);
void setVerboseLogging(bool enabled);
