#include "interaction_system.h"
#include "math_utils.h"
#include "constants.h"
#include "collision_system.h"  // For checkPointInBounds
#include <cmath>

void handleInteractions(Camera3D& camera, EnvironmentManager& environment, GameState& state, float currentTime) {
    bool nearInteractable = false;
    std::string interactableName = "";
    static float lastEPressTime = 0.0f;

    // **PHASE 2 ENHANCEMENT**: Interaction input using enhanced input with built-in debouncing
    bool eKeyPressed = state.enhancedInput.isActionPressed("interact") && (currentTime - lastEPressTime) > 0.3f;

    if (eKeyPressed) {
        lastEPressTime = currentTime;
    }

    // Building interactions are now handled below with priority logic

    // Check for NPC interactions first (higher priority when inside building)
    bool interactingWithNPC = false;
    for (int n = 0; n < MAX_NPCS; n++) {
        bool npcVisible = false;

        if (state.isInBuilding) {
            if (state.currentBuilding == 0 && n == 0) npcVisible = true;
            if (state.currentBuilding == 1 && n == 1) npcVisible = true;
        }

        if (npcVisible) {
            float distance = MathUtils::distance3D(npcs[n].position, camera.position);

            if (distance <= npcs[n].interactionRadius * 1.5f) {
                nearInteractable = true;
                interactableName = "Press E to talk to " + npcs[n].name;
                if (eKeyPressed && !state.isInDialog) {
                    startDialog(n, state);
                    state.testNPCInteraction = true;
                    interactingWithNPC = true;  // Flag that we're interacting with NPC
                    break;  // Exit the NPC loop once we find one to interact with
                }
            }
        }
    }

    // Only check door interactions if we're not interacting with an NPC
    if (!interactingWithNPC) {
        auto objects = environment.getAllObjects();
        for (size_t i = 0; i < objects.size(); ++i) {
            if (auto building = std::dynamic_pointer_cast<Building>(objects[i])) {
                if (building->isInteractive()) {
                    Vector3 doorPos = building->getDoorPosition();
                    float distance = MathUtils::distance3D(doorPos, camera.position);

                    if (distance <= EnvironmentConstants::INTERACTION_DISTANCE && !state.isInBuilding) {
                        nearInteractable = true;
                        interactableName = "Press E to enter " + building->getName();
                        if (eKeyPressed) {
                            state.isInBuilding = true;
                            state.currentBuilding = building->getId();  // Use building ID instead of array index
                            state.lastOutdoorPosition = camera.position;

                            // Set position more centered in the building interior
                            camera.position = {building->position.x, 1.75f, building->position.z};
                            camera.target = {building->position.x, 1.55f, building->position.z - 3.0f}; // Look toward back of building
                            state.playerY = 0.0f;
                            state.testBuildingEntry = true;

                            printf("Entered building: %s (ID: %d) at position (%.1f, %.1f, %.1f)\n",
                                   building->getName().c_str(), building->getId(),
                                   camera.position.x, camera.position.y, camera.position.z);
                            break;
                        }
                    } else if (state.isInBuilding && state.currentBuilding == building->getId() && eKeyPressed) {
                        state.isInBuilding = false;
                        camera.position = state.lastOutdoorPosition;
                        camera.target = {state.lastOutdoorPosition.x, state.lastOutdoorPosition.y - 0.2f, state.lastOutdoorPosition.z - 5.0f};
                        state.currentBuilding = -1;
                        printf("Exited building: %s\n", building->getName().c_str());
                        break;
                    }
                }
            }
        }
    }

    static float lastInteractionTime = 0.0f;
    static bool wasNearInteractable = false;

    if (nearInteractable) {
        state.showInteractPrompt = true;
        state.interactPromptText = interactableName;
        lastInteractionTime = currentTime;
        wasNearInteractable = true;
    } else {
        if (wasNearInteractable && (currentTime - lastInteractionTime) > 0.5f) {
            state.showInteractPrompt = false;
            wasNearInteractable = false;
        }
        if (eKeyPressed && state.showInteractPrompt) {
            lastInteractionTime = currentTime;
        }
    }

    if (state.isInBuilding) {
        state.testBuildingEntry = true;
    }

    if (state.showInteractPrompt && state.interactPromptText.find("talk to") != std::string::npos) {
        state.testNPCInteraction = true;
    }

    if (state.isInDialog && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();

        const int buttonY = 320;
        const int buttonHeight = 30;
        const int buttonWidth = 150;
        const int buttonSpacing = 170;
        const int startX = 50;

        for (int i = 0; i < state.numDialogOptions; i++) {
            int buttonX = startX + (i * buttonSpacing);
            int buttonRight = buttonX + buttonWidth;
            int buttonBottom = buttonY + buttonHeight;

            int hitPadding = 5;
            if (mousePos.x >= buttonX - hitPadding && mousePos.x <= buttonRight + hitPadding &&
                mousePos.y >= buttonY - hitPadding && mousePos.y <= buttonBottom + hitPadding) {
                handleDialogOption(i, state);
                break;
            }
        }
    }
}

// NEW: Update building entry based on position
void updateBuildingEntry(Camera3D& camera, GameState& state, EnvironmentManager& environment) {
    bool playerInBuilding = false;
    int buildingIndex = -1;

    auto objects = environment.getAllObjects();
    for (size_t i = 0; i < objects.size(); ++i) {
        if (auto building = std::dynamic_pointer_cast<Building>(objects[i])) {
            CollisionBounds bBounds = building->getCollisionBounds();  // Assume getCollisionBounds() exists or add to EnvironmentalObject
            if (CollisionSystem::checkPointInBounds(camera.position, bBounds)) {
                playerInBuilding = true;
                buildingIndex = building->getId();
                break;
            }
        }
    }

    if (playerInBuilding && !state.isInBuilding) {
        state.isInBuilding = true;
        state.currentBuilding = buildingIndex;
        state.enhancedInput.setMouseCaptured(false);
        state.mouseReleased = true;
        printf("Detected player inside building via position: %s\n", 
               objects[buildingIndex]->getName().c_str());
        printf("Mouse released for building interaction\n");
    } else if (!playerInBuilding && state.isInBuilding) {
        state.isInBuilding = false;
        state.currentBuilding = -1;
        state.enhancedInput.setMouseCaptured(true);
        state.mouseReleased = false;
        printf("Player exited building\n");
        printf("Mouse captured for FPS controls\n");
    }
}