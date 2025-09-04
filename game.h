#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "game_state.h"  // For GameState
#include "config.h"      // For GameConfig
#include "input_manager.h"  // For InputManager and EnhancedInputManager
#include "environment_manager.h"  // For EnvironmentManager
#include "performance.h"  // For AdvancedFrameStats
#include "menu_system.h"  // For MenuSystem
#include "render_system.h"  // NEW

// Add other includes if needed for types in declarations (e.g., InventorySystem if defined elsewhere)

class Game {
public:
    Game();
    ~Game();

    // Main run function
    int Run();

private:
    // Initialization
    void Init();
    void InitWindowAndConfig();
    void InitSystems();
    void InitWorldAndEntities();

    // Main loop phases
    void Update(float deltaTime);
    void HandleInput(float deltaTime);
    void UpdateSystems(float deltaTime);
    void Render();

    // Shutdown
    void Shutdown();

    // Member variables for systems and state
    GameConfig config;
    Camera3D camera;
    InputManager input;  // Legacy input (keep for compatibility)
    EnhancedInputManager enhancedInput;  // Assuming this exists based on usage
    GameState state;
    EnvironmentManager environment;
    AdvancedFrameStats performanceStats;
    std::unique_ptr<InventorySystem> inventorySystem;  // Managed by state, but here for clarity
    std::unique_ptr<MenuSystem> menuSystem_;  // NEW: For menu input handling
    std::unique_ptr<RenderSystem> renderSystem_;  // NEW: For modular rendering

    // Flags
    bool shouldClose = false;
    int frameCounter = 0;
};

#endif // GAME_H