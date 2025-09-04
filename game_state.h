// game_state.h (updated)
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "raylib.h"
#include "inventory.h"
#include "input_manager.h"
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <memory>

struct PerformanceMetrics {
    float averageFrameTime = 0.0f;
    int totalFrames = 0;
    std::chrono::steady_clock::time_point sessionStart = std::chrono::steady_clock::now();
    std::vector<float> frameTimeHistory;  // Last N frame times (e.g., limit to 100 for recent history)
};

struct GameState {
    bool mouseReleased = false;

    // Dialog state
    bool isInDialog = false;
    int currentNPC = -1;
    std::string dialogText = "";
    std::string dialogOptions[3] = {"", "", ""};
    int numDialogOptions = 0;
    bool showDialogWindow = false;
    
    // Inventory UI state
    bool showInventoryWindow = false;
    std::string lastClickedItem = "";  // For inventory item interactions
    std::string inventorySearchQuery = "";  // Current search query
    bool inventorySearchActive = false;  // Whether search is active
    
    // ESC Menu system
    bool showEscMenu = false;
    int selectedMenuOption = 0;
    bool showTestingPanel = false;  // **TAB KEY TOGGLE** - Detailed testing checklist  // 0=Resume, 1=Save, 2=Load, 3=Quit

    // Game state
    bool isInBuilding = false;
    int currentBuilding = -1;
    bool showInteractPrompt = false;
    std::string interactPromptText = "";
    Vector3 lastOutdoorPosition = {0.0f, 1.75f, 0.0f};

    // Player state
    float playerY = 0.0f;
    bool isJumping = false;
    bool isGrounded = true;
    float jumpVelocity = 0.0f;
    
    // Player stats and RPG mechanics
    int playerHealth = 100;
    int maxPlayerHealth = 100;
    int playerMana = 50;
    int maxPlayerMana = 50;
    int playerStamina = 100;
    int maxPlayerStamina = 100;
    int playerLevel = 1;
    int playerExperience = 0;
    
    // Inventory system (pointer to Game's owned inventory)
    InventorySystem* inventorySystem = nullptr;

    // Game control flags
    bool shouldClose = false;

    // **PHASE 2 ENHANCEMENT**: Enhanced Input Manager for centralized input handling
    mutable EnhancedInputManager enhancedInput;

    // Combat timing
    float lastSwingTime = 0.0f;
    float swingCooldown = 0.5f; // 500ms between swings

    // Stats
    int swingsPerformed = 0;
    int meleeHits = 0;
    int score = 0;

    // Testing state
    bool testMouseCaptured = false;
    bool testBuildingCollision = false;
    bool testWASDMovement = false;
    bool testSpaceJump = false;
    bool testMouseLook = false;
    bool testMeleeSwing = false;
    bool testMeleeHitDetection = false;
    bool testBuildingEntry = false;
    bool testNPCInteraction = false;
    Vector3 lastCameraPos = {0.0f, 0.0f, 0.0f};  // Initialized to zero or camera start

    // New: Performance metrics
    PerformanceMetrics metrics;

    // New: State validation methods
    bool isValid() const;
    void validateAndRepair();
    void resetToDefaults();

    // New: State change notifications
    using StateChangeCallback = std::function<void(const std::string& property)>;
    void addChangeListener(StateChangeCallback callback);
    void notifyChange(const std::string& property);  // Call this when changing state properties
    
    // New: Save/Load convenience methods
    bool saveState(const std::string& filename = "browserwind_save.dat") const;
    bool loadState(const std::string& filename = "browserwind_save.dat");

private:
    std::vector<StateChangeCallback> changeListeners_;
    // **PHASE 2 FIX**: Removed mutex - single-threaded game doesn't need thread safety
};

// New: Serialization functions
bool saveState(const GameState& state, const std::string& filename);
bool loadState(GameState& state, const std::string& filename);

#endif