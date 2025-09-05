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
#include <unordered_map>

struct PerformanceMetrics {
    float averageFrameTime = 0.0f;
    int totalFrames = 0;
    std::chrono::steady_clock::time_point sessionStart = std::chrono::steady_clock::now();
    std::vector<float> frameTimeHistory;  // Last N frame times (e.g., limit to 100 for recent history)
};

/// \brief Versioned serializer for game state with migration support.
class VersionedSerializer {
public:
    static constexpr uint32_t CURRENT_VERSION = 1;

    /// \brief Serializes an object to stream.
    /// \tparam T Type of object to serialize.
    /// \param obj Object to serialize.
    /// \param stream Output stream.
    /// \return True on success.
    template<typename T>
    bool serialize(const T& obj, std::ostream& stream);

    /// \brief Deserializes an object from stream.
    /// \tparam T Type of object to deserialize.
    /// \param obj Object to fill.
    /// \param stream Input stream.
    /// \return True on success.
    template<typename T>
    bool deserialize(T& obj, std::istream& stream);

    /// \brief Registers a migration function for a version.
    /// \param fromVersion Version to migrate from.
    /// \param migrator Migration function.
    void registerMigrator(uint32_t fromVersion, std::function<void(std::istream&)> migrator);

private:
    std::unordered_map<uint32_t, std::function<void(std::istream&)>> migrators_;
};

/// \brief Main game state struct.
/// This holds all runtime game state data. To address SRP, consider splitting into sub-structs (e.g., PlayerState, UIState) in future.
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
    /// \brief Checks if state is valid.
    /// \return True if valid.
    bool isValid() const;

    /// \brief Validates and repairs invalid state.
    void validateAndRepair();

    /// \brief Resets state to defaults.
    void resetToDefaults();

    // New: State change notifications
    using StateChangeCallback = std::function<void(const std::string& property)>;
    /// \brief Adds a change listener.
    /// \param callback Callback to add.
    void addChangeListener(StateChangeCallback callback);

    /// \brief Notifies listeners of change.
    /// \param property Changed property.
    void notifyChange(const std::string& property);
    
    // New: Save/Load convenience methods
    /// \brief Saves state to file.
    /// \param filename File to save to.
    /// \return True on success.
    bool saveState(const std::string& filename = "browserwind_save.dat") const;

    /// \brief Loads state from file.
    /// \param filename File to load from.
    /// \return True on success.
    bool loadState(const std::string& filename = "browserwind_save.dat");

private:
    std::vector<StateChangeCallback> changeListeners_;
};

// Standalone serialization functions (using VersionedSerializer internally)
bool saveState(const GameState& state, const std::string& filename);
bool loadState(GameState& state, const std::string& filename);

#endif