#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "game_state.h"  // For GameState
#include "config.h"      // For GameConfig
#include "input_manager.h"  // For InputManager and EnhancedInputManager
#include "environment_manager.h"  // For EnvironmentManager
#include "menu_system.h"  // For MenuSystem
#include "render_system.h"  // For RenderSystem

// ECS components are defined in environmental_object.h
// Include that header when ECS functionality is needed

class Entity {
public:
    void addComponent(std::unique_ptr<Component> component);
    Component* getComponent(const std::string& typeName) const;
private:
    std::unordered_map<std::string, std::unique_ptr<Component>> components_;
};

// Simple performance stats (enhanced with chrono for accurate timing)
#include <chrono>

struct SimplePerformanceStats {
    float averageFrameTime = 0.0f;
    int frameCount = 0;
    bool showDetailedStats = false;

    struct Timer {
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
        void start() { startTime = std::chrono::steady_clock::now(); }
        void end() { endTime = std::chrono::steady_clock::now(); }
        double elapsed() const {
            return std::chrono::duration<double, std::milli>(endTime - startTime).count();
        }
    } renderingTimer;
};

// Add other includes if needed for types in declarations (e.g., InventorySystem if defined elsewhere)

/// \brief Main game class managing initialization, update loop, and shutdown.
/// This class serves as the entry point and coordinator for all game systems.
class Game {
public:
    /// \brief Default constructor.
    Game();

    /// \brief Destructor, ensures proper shutdown.
    ~Game();

    /// \brief Main game loop runner.
    /// \return EXIT_SUCCESS on clean exit, EXIT_FAILURE on error.
    int Run();

private:
    // Initialization methods
    /// \brief Overall initialization handler.
    void Init();

    /// \brief Initializes window and loads configuration.
    void InitWindowAndConfig();

    /// \brief Initializes all game systems.
    void InitSystems();

    /// \brief Initializes world, entities, and ECS foundation.
    void InitWorldAndEntities();

    // Main loop phases
    /// \brief Updates game state.
    /// \param deltaTime Time since last frame.
    void Update(float deltaTime);

    /// \brief Handles input processing.
    /// \param deltaTime Time since last frame.
    void HandleInput(float deltaTime);

    /// \brief Updates all subsystems.
    /// \param deltaTime Time since last frame.
    void UpdateSystems(float deltaTime);

    /// \brief Renders the current frame.
    void Render();

    // Shutdown
    /// \brief Cleans up all resources.
    void Shutdown();

    // Member variables (using smart pointers for ownership)
    GameConfig config_;
    Camera3D camera_;
    std::unique_ptr<InputManager> input_;  // Legacy input (unique_ptr for RAII)
    std::unique_ptr<EnhancedInputManager> enhancedInput_;  // Enhanced input
    GameState state_;
    std::unique_ptr<EnvironmentManager> environment_;  // Owned environment
    SimplePerformanceStats performanceStats_;  // Simple performance stats
    std::unique_ptr<InventorySystem> inventorySystem_;  // Owned inventory
    std::unique_ptr<MenuSystem> menuSystem_;  // Owned menu system
    std::unique_ptr<RenderSystem> renderSystem_;  // Owned render system

    // Flags and counters
    bool shouldClose_ = false;
    int frameCounter_ = 0;

    // ECS preparation (to be migrated fully)
    std::vector<std::unique_ptr<Entity>> entities_;  // Basic entity list for migration
};

#endif // GAME_H