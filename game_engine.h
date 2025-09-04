// game_engine.h - Modern Game Engine Architecture
#pragma once

#include "ui_system.h"
#include "simple_inventory.h"
#include "input_manager.h"
#include "environment_manager.h"
#include "game_state.h"
#include "performance.h"
#include "npc.h"
#include <memory>

// ===== MODERN GAME ENGINE CLASS =====
// Eliminates global variables through dependency injection pattern
// Following modern C++ game development best practices

class GameEngine {
public:
    // Constructor - Initialize all systems
    GameEngine();
    
    // Destructor - Clean shutdown
    ~GameEngine();
    
    // Main game loop - replaces scattered global logic
    int run();
    
    // System access (dependency injection)
    UISystemManager& getUISystem() { return *uiSystem_; }
    SimpleInventory& getInventory() { return *inventory_; }
    InputManager& getInputManager() { return *inputManager_; }
    EnvironmentManager& getEnvironmentManager() { return *environmentManager_; }
    
private:
    // System ownership - no more globals!
    std::unique_ptr<UISystemManager> uiSystem_;
    std::unique_ptr<SimpleInventory> inventory_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<EnvironmentManager> environmentManager_;
    
    // Game state and performance tracking
    std::unique_ptr<GameState> gameState_;
    std::unique_ptr<AdvancedFrameStats> performanceStats_;
    
    // Core game objects
    std::vector<NPC> npcs_;
    Camera3D camera_;
    
    // Private methods for clean game loop phases
    void processInput(bool& shouldClose);
    void updateGameLogic(float deltaTime);
    void renderGame();
    
    // Initialization methods
    void initializeSystems();
    void initializeWorld();
    void initializeCamera();
    
    // Cleanup
    void shutdown();
    
    // Performance validation
    void validatePerformance() const;
};

// Helper structure for NPC visibility (moved from main.cpp)
struct NPCVisibilityState {
    bool shouldDraw[MAX_NPCS] = {false};
};
