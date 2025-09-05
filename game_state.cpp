#include "game_state.h"
#include <fstream>
#include <iostream>  // For debugging/error messages
#include <algorithm>  // For std::min/std::max if needed in validation

// Helper functions for binary serialization of strings and vectors
void writeString(std::ostream& stream, const std::string& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    stream.write(reinterpret_cast<const char*>(&len), sizeof(len));
    stream.write(str.data(), len);
}

std::string readString(std::istream& stream) {
    uint32_t len;
    stream.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string str(len, '\0');
    stream.read(&str[0], len);
    return str;
}

template<typename T>
bool VersionedSerializer::serialize(const T& obj, std::ostream& stream) {
    uint32_t version = CURRENT_VERSION;
    stream.write(reinterpret_cast<const char*>(&version), sizeof(version));
    // Custom serialization for GameState (manual for now)
    return true;  // Placeholder; implement per-type
}

template<typename T>
bool VersionedSerializer::deserialize(T& obj, std::istream& stream) {
    uint32_t version;
    stream.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version > CURRENT_VERSION) {
        return false;  // Future version unsupported
    }
    if (auto it = migrators_.find(version); it != migrators_.end()) {
        it->second(stream);
    }
    // Custom deserialization
    return true;
}

void VersionedSerializer::registerMigrator(uint32_t fromVersion, std::function<void(std::istream&)> migrator) {
    migrators_[fromVersion] = std::move(migrator);
}

bool GameState::isValid() const {
    // Basic validation checks - expand as needed
    if (currentNPC < -1 || numDialogOptions < 0 || numDialogOptions > 3) return false;
    if (currentBuilding < -1) return false;
    if (playerY < 0.0f || jumpVelocity < 0.0f) return false;  // Assuming non-negative for these
    if (swingsPerformed < 0 || meleeHits < 0 || score < 0) return false;
    
    // Player stats validation
    if (playerHealth < 0 || playerHealth > maxPlayerHealth) return false;
    if (playerMana < 0 || playerMana > maxPlayerMana) return false;
    if (playerStamina < 0 || playerStamina > maxPlayerStamina) return false;
    if (playerLevel < 1 || playerExperience < 0) return false;
    if (metrics.totalFrames < 0 || metrics.averageFrameTime < 0.0f) return false;
    // Add more checks for your specific invariants
    return true;
}

void GameState::validateAndRepair() {
    // Repair invalid states
    if (currentNPC < -1) currentNPC = -1;
    if (numDialogOptions < 0) numDialogOptions = 0;
    if (numDialogOptions > 3) numDialogOptions = 3;
    if (currentBuilding < -1) currentBuilding = -1;
    if (playerY < 0.0f) playerY = 0.0f;
    if (jumpVelocity < 0.0f) jumpVelocity = 0.0f;
    if (swingsPerformed < 0) swingsPerformed = 0;
    if (meleeHits < 0) meleeHits = 0;
    if (score < 0) score = 0;
    
    // Player stats validation and repair
    if (playerHealth < 0) playerHealth = 0;
    if (playerHealth > maxPlayerHealth) playerHealth = maxPlayerHealth;
    if (playerMana < 0) playerMana = 0;
    if (playerMana > maxPlayerMana) playerMana = maxPlayerMana;
    if (playerStamina < 0) playerStamina = 0;
    if (playerStamina > maxPlayerStamina) playerStamina = maxPlayerStamina;
    if (playerLevel < 1) playerLevel = 1;
    if (playerExperience < 0) playerExperience = 0;
    if (metrics.totalFrames < 0) metrics.totalFrames = 0;
    if (metrics.averageFrameTime < 0.0f) metrics.averageFrameTime = 0.0f;
    // Clear invalid dialog options if needed
    for (int i = numDialogOptions; i < 3; ++i) {
        dialogOptions[i] = "";
    }
    notifyChange("validated");
}

void GameState::resetToDefaults() {
    // Reset all members to default values
    mouseReleased = false;
    isInDialog = false;
    currentNPC = -1;
    dialogText = "";
    for (int i = 0; i < 3; ++i) dialogOptions[i] = "";
    numDialogOptions = 0;
    showDialogWindow = false;
    isInBuilding = false;
    currentBuilding = -1;
    showInteractPrompt = false;
    interactPromptText = "";
    lastOutdoorPosition = {0.0f, 1.75f, 0.0f};
    playerY = 0.0f;
    isJumping = false;
    isGrounded = true;
    jumpVelocity = 0.0f;
    
    // Reset player stats to defaults
    playerHealth = 100;
    maxPlayerHealth = 100;
    playerMana = 50;
    maxPlayerMana = 50;
    playerStamina = 100;
    maxPlayerStamina = 100;
    playerLevel = 1;
    playerExperience = 0;
    
    // Reset inventory system (will be recreated when needed)
    inventorySystem = nullptr;
    
    lastSwingTime = 0.0f;
    swingCooldown = 0.5f;
    swingsPerformed = 0;
    meleeHits = 0;
    score = 0;
    testMouseCaptured = false;
    testBuildingCollision = false;
    testWASDMovement = false;
    testSpaceJump = false;
    testMouseLook = false;
    testMeleeSwing = false;
    testMeleeHitDetection = false;
    testBuildingEntry = false;
    testNPCInteraction = false;
    lastCameraPos = {0.0f, 0.0f, 0.0f};
    metrics = PerformanceMetrics{};
    changeListeners_.clear();
    notifyChange("reset");
}

void GameState::addChangeListener(StateChangeCallback callback) {
    changeListeners_.push_back(callback);
}

void GameState::notifyChange(const std::string& property) {
    for (const auto& listener : changeListeners_) {
        listener(property);
    }
}

bool saveState(const GameState& state, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open save file: " << filename << std::endl;
        return false;
    }

    uint32_t version = VersionedSerializer::CURRENT_VERSION;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // Binary write for all fields (manual)
    file.write(reinterpret_cast<const char*>(&state.mouseReleased), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.isInDialog), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.currentNPC), sizeof(int));
    writeString(file, state.dialogText);
    file.write(reinterpret_cast<const char*>(&state.numDialogOptions), sizeof(int));
    for (int i = 0; i < 3; ++i) {
        writeString(file, state.dialogOptions[i]);
    }
    file.write(reinterpret_cast<const char*>(&state.showDialogWindow), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.isInBuilding), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.currentBuilding), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.showInteractPrompt), sizeof(bool));
    writeString(file, state.interactPromptText);
    file.write(reinterpret_cast<const char*>(&state.lastOutdoorPosition), sizeof(Vector3));
    file.write(reinterpret_cast<const char*>(&state.playerY), sizeof(float));
    file.write(reinterpret_cast<const char*>(&state.isJumping), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.isGrounded), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.jumpVelocity), sizeof(float));

    // Player stats
    file.write(reinterpret_cast<const char*>(&state.playerHealth), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.maxPlayerHealth), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.playerMana), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.maxPlayerMana), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.playerStamina), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.maxPlayerStamina), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.playerLevel), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.playerExperience), sizeof(int));

    file.write(reinterpret_cast<const char*>(&state.lastSwingTime), sizeof(float));
    file.write(reinterpret_cast<const char*>(&state.swingsPerformed), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.meleeHits), sizeof(int));
    file.write(reinterpret_cast<const char*>(&state.score), sizeof(int));

    // Testing states
    file.write(reinterpret_cast<const char*>(&state.testMouseCaptured), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testBuildingCollision), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testWASDMovement), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testSpaceJump), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testMouseLook), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testMeleeSwing), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testMeleeHitDetection), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testBuildingEntry), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.testNPCInteraction), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&state.lastCameraPos), sizeof(Vector3));

    // Performance metrics (simple; skip frameTimeHistory for now)
    file.write(reinterpret_cast<const char*>(&state.metrics.averageFrameTime), sizeof(float));
    file.write(reinterpret_cast<const char*>(&state.metrics.totalFrames), sizeof(int));

    if (!file.good()) {
        std::cerr << "Error writing save file" << std::endl;
        return false;
    }
    return true;
}

bool loadState(GameState& state, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open load file: " << filename << std::endl;
        state.resetToDefaults();  // Error recovery: reset to defaults
        return false;
    }

    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != VersionedSerializer::CURRENT_VERSION) {
        std::cerr << "Unsupported save version: " << version << std::endl;
        state.resetToDefaults();  // Error recovery
        return false;
    }

    // Binary read for all fields
    file.read(reinterpret_cast<char*>(&state.mouseReleased), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.isInDialog), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.currentNPC), sizeof(int));
    state.dialogText = readString(file);
    file.read(reinterpret_cast<char*>(&state.numDialogOptions), sizeof(int));
    for (int i = 0; i < 3; ++i) {
        state.dialogOptions[i] = readString(file);
    }
    file.read(reinterpret_cast<char*>(&state.showDialogWindow), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.isInBuilding), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.currentBuilding), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.showInteractPrompt), sizeof(bool));
    state.interactPromptText = readString(file);
    file.read(reinterpret_cast<char*>(&state.lastOutdoorPosition), sizeof(Vector3));
    file.read(reinterpret_cast<char*>(&state.playerY), sizeof(float));
    file.read(reinterpret_cast<char*>(&state.isJumping), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.isGrounded), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.jumpVelocity), sizeof(float));

    // Player stats
    file.read(reinterpret_cast<char*>(&state.playerHealth), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.maxPlayerHealth), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.playerMana), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.maxPlayerMana), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.playerStamina), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.maxPlayerStamina), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.playerLevel), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.playerExperience), sizeof(int));

    file.read(reinterpret_cast<char*>(&state.lastSwingTime), sizeof(float));
    file.read(reinterpret_cast<char*>(&state.swingsPerformed), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.meleeHits), sizeof(int));
    file.read(reinterpret_cast<char*>(&state.score), sizeof(int));

    // Testing states
    file.read(reinterpret_cast<char*>(&state.testMouseCaptured), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testBuildingCollision), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testWASDMovement), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testSpaceJump), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testMouseLook), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testMeleeSwing), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testMeleeHitDetection), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testBuildingEntry), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.testNPCInteraction), sizeof(bool));
    file.read(reinterpret_cast<char*>(&state.lastCameraPos), sizeof(Vector3));

    // Performance metrics
    file.read(reinterpret_cast<char*>(&state.metrics.averageFrameTime), sizeof(float));
    file.read(reinterpret_cast<char*>(&state.metrics.totalFrames), sizeof(int));

    if (!file.good()) {
        std::cerr << "Error reading save file" << std::endl;
        state.resetToDefaults();  // Error recovery
        return false;
    }

    // Validate after load
    state.validateAndRepair();
    return true;
}

// **CONVENIENCE METHODS** - Wrapper methods for easier save/load
bool GameState::saveState(const std::string& filename) const {
    return ::saveState(*this, filename);  // Call the standalone function
}

bool GameState::loadState(const std::string& filename) {
    return ::loadState(*this, filename);  // Call the standalone function
}