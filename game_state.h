// game_state.h
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "raylib.h"
#include <string>

struct GameState {
    bool mouseReleased = false;

    // Dialog state
    bool isInDialog = false;
    int currentNPC = -1;
    std::string dialogText = "";
    std::string dialogOptions[3] = {"", "", ""};
    int numDialogOptions = 0;
    bool showDialogWindow = false;

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
};

#endif