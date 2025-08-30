// game_state.cpp (updated)
#include "game_state.h"
#include <fstream>
#include <iostream>  // For debugging/error messages
#include <algorithm>  // For std::min/std::max if needed in validation

bool GameState::isValid() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
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
    std::lock_guard<std::mutex> lock(stateMutex_);
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
    // Notify if repairs were made (optional)
    notifyChange("validated");
}

void GameState::resetToDefaults() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    // Manually reset all members since copy assignment is disabled due to mutex
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
    std::lock_guard<std::mutex> lock(stateMutex_);
    changeListeners_.push_back(callback);
}

void GameState::notifyChange(const std::string& property) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    for (const auto& listener : changeListeners_) {
        listener(property);
    }
}

bool saveState(const GameState& state, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open save file: " << filename << std::endl;
        return false;
    }

    // Save simple key-value pairs (expand for all members)
    file << "mouseReleased=" << state.mouseReleased << "\n";
    file << "isInDialog=" << state.isInDialog << "\n";
    file << "currentNPC=" << state.currentNPC << "\n";
    file << "dialogText=" << state.dialogText << "\n";  // Note: Escape if contains newlines
    file << "numDialogOptions=" << state.numDialogOptions << "\n";
    for (int i = 0; i < 3; ++i) {
        file << "dialogOption" << i << "=" << state.dialogOptions[i] << "\n";
    }
    file << "showDialogWindow=" << state.showDialogWindow << "\n";
    file << "isInBuilding=" << state.isInBuilding << "\n";
    file << "currentBuilding=" << state.currentBuilding << "\n";
    file << "showInteractPrompt=" << state.showInteractPrompt << "\n";
    file << "interactPromptText=" << state.interactPromptText << "\n";
    file << "lastOutdoorPositionX=" << state.lastOutdoorPosition.x << "\n";
    file << "lastOutdoorPositionY=" << state.lastOutdoorPosition.y << "\n";
    file << "lastOutdoorPositionZ=" << state.lastOutdoorPosition.z << "\n";
    file << "playerY=" << state.playerY << "\n";
    file << "isJumping=" << state.isJumping << "\n";
    file << "isGrounded=" << state.isGrounded << "\n";
    file << "jumpVelocity=" << state.jumpVelocity << "\n";
    
    // Player stats
    file << "playerHealth=" << state.playerHealth << "\n";
    file << "maxPlayerHealth=" << state.maxPlayerHealth << "\n";
    file << "playerMana=" << state.playerMana << "\n";
    file << "maxPlayerMana=" << state.maxPlayerMana << "\n";
    file << "playerStamina=" << state.playerStamina << "\n";
    file << "maxPlayerStamina=" << state.maxPlayerStamina << "\n";
    file << "playerLevel=" << state.playerLevel << "\n";
    file << "playerExperience=" << state.playerExperience << "\n";
    
    file << "lastSwingTime=" << state.lastSwingTime << "\n";
    file << "swingsPerformed=" << state.swingsPerformed << "\n";
    file << "meleeHits=" << state.meleeHits << "\n";
    file << "score=" << state.score << "\n";
    // Testing states
    file << "testMouseCaptured=" << state.testMouseCaptured << "\n";
    file << "testBuildingCollision=" << state.testBuildingCollision << "\n";
    file << "testWASDMovement=" << state.testWASDMovement << "\n";
    file << "testSpaceJump=" << state.testSpaceJump << "\n";
    file << "testMouseLook=" << state.testMouseLook << "\n";
    file << "testMeleeSwing=" << state.testMeleeSwing << "\n";
    file << "testMeleeHitDetection=" << state.testMeleeHitDetection << "\n";
    file << "testBuildingEntry=" << state.testBuildingEntry << "\n";
    file << "testNPCInteraction=" << state.testNPCInteraction << "\n";
    file << "lastCameraPosX=" << state.lastCameraPos.x << "\n";
    file << "lastCameraPosY=" << state.lastCameraPos.y << "\n";
    file << "lastCameraPosZ=" << state.lastCameraPos.z << "\n";
    // Performance metrics (simple for now; frameTimeHistory could be serialized as comma-separated)
    file << "averageFrameTime=" << state.metrics.averageFrameTime << "\n";
    file << "totalFrames=" << state.metrics.totalFrames << "\n";
    // Note: sessionStart not saved (restarts on load); frameTimeHistory not saved for simplicity

    return true;
}

bool loadState(GameState& state, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open load file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "mouseReleased") state.mouseReleased = (value == "1");
        else if (key == "isInDialog") state.isInDialog = (value == "1");
        else if (key == "currentNPC") state.currentNPC = std::stoi(value);
        else if (key == "dialogText") state.dialogText = value;
        else if (key == "numDialogOptions") state.numDialogOptions = std::stoi(value);
        else if (key == "dialogOption0") state.dialogOptions[0] = value;
        else if (key == "dialogOption1") state.dialogOptions[1] = value;
        else if (key == "dialogOption2") state.dialogOptions[2] = value;
        else if (key == "showDialogWindow") state.showDialogWindow = (value == "1");
        else if (key == "isInBuilding") state.isInBuilding = (value == "1");
        else if (key == "currentBuilding") state.currentBuilding = std::stoi(value);
        else if (key == "showInteractPrompt") state.showInteractPrompt = (value == "1");
        else if (key == "interactPromptText") state.interactPromptText = value;
        else if (key == "lastOutdoorPositionX") state.lastOutdoorPosition.x = std::stof(value);
        else if (key == "lastOutdoorPositionY") state.lastOutdoorPosition.y = std::stof(value);
        else if (key == "lastOutdoorPositionZ") state.lastOutdoorPosition.z = std::stof(value);
        else if (key == "playerY") state.playerY = std::stof(value);
        else if (key == "isJumping") state.isJumping = (value == "1");
        else if (key == "isGrounded") state.isGrounded = (value == "1");
        else if (key == "jumpVelocity") state.jumpVelocity = std::stof(value);
        
        // Player stats loading
        else if (key == "playerHealth") state.playerHealth = std::stoi(value);
        else if (key == "maxPlayerHealth") state.maxPlayerHealth = std::stoi(value);
        else if (key == "playerMana") state.playerMana = std::stoi(value);
        else if (key == "maxPlayerMana") state.maxPlayerMana = std::stoi(value);
        else if (key == "playerStamina") state.playerStamina = std::stoi(value);
        else if (key == "maxPlayerStamina") state.maxPlayerStamina = std::stoi(value);
        else if (key == "playerLevel") state.playerLevel = std::stoi(value);
        else if (key == "playerExperience") state.playerExperience = std::stoi(value);
        
        else if (key == "lastSwingTime") state.lastSwingTime = std::stof(value);
        else if (key == "swingsPerformed") state.swingsPerformed = std::stoi(value);
        else if (key == "meleeHits") state.meleeHits = std::stoi(value);
        else if (key == "score") state.score = std::stoi(value);
        else if (key == "testMouseCaptured") state.testMouseCaptured = (value == "1");
        else if (key == "testBuildingCollision") state.testBuildingCollision = (value == "1");
        else if (key == "testWASDMovement") state.testWASDMovement = (value == "1");
        else if (key == "testSpaceJump") state.testSpaceJump = (value == "1");
        else if (key == "testMouseLook") state.testMouseLook = (value == "1");
        else if (key == "testMeleeSwing") state.testMeleeSwing = (value == "1");
        else if (key == "testMeleeHitDetection") state.testMeleeHitDetection = (value == "1");
        else if (key == "testBuildingEntry") state.testBuildingEntry = (value == "1");
        else if (key == "testNPCInteraction") state.testNPCInteraction = (value == "1");
        else if (key == "lastCameraPosX") state.lastCameraPos.x = std::stof(value);
        else if (key == "lastCameraPosY") state.lastCameraPos.y = std::stof(value);
        else if (key == "lastCameraPosZ") state.lastCameraPos.z = std::stof(value);
        else if (key == "averageFrameTime") state.metrics.averageFrameTime = std::stof(value);
        else if (key == "totalFrames") state.metrics.totalFrames = std::stoi(value);
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