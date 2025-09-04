// Minimal debug system - stubs only
#include "debug_system.h"

// Global variables that other files expect
int DEBUG_LEVEL = 0;
bool DEBUG_MODE = false;

// Stub implementations - do nothing for performance
void DebugSystem::initialize() {}
void DebugSystem::log([[maybe_unused]] DebugCategory category, [[maybe_unused]] int level, [[maybe_unused]] const char* file, [[maybe_unused]] int line, [[maybe_unused]] const char* function, [[maybe_unused]] const std::string& message) {}
void DebugSystem::traceEnter([[maybe_unused]] const char* functionName) {}
void DebugSystem::traceExit([[maybe_unused]] const char* functionName) {}
void DebugSystem::setLastKnownLocation([[maybe_unused]] const std::string& location) {}
void DebugSystem::startFrameProfile() {}
void DebugSystem::endFrameProfile() {}
void DebugSystem::generatePerformanceReport() {}
