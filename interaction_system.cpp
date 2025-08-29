// interaction_system.cpp
#include "interaction_system.h"
#include <cmath>

void handleInteractions(Camera3D& camera, EnvironmentManager& environment, GameState& state, float currentTime) {
    bool nearInteractable = false;
    std::string interactableName = "";
    static float lastEPressTime = 0.0f;

    bool eKeyPressed = IsKeyPressed(KEY_E) && (currentTime - lastEPressTime) > 0.3f;

    if (eKeyPressed) {
        lastEPressTime = currentTime;
    }

    auto objects = environment.getAllObjects();
    for (size_t i = 0; i < objects.size(); ++i) {
        if (auto building = std::dynamic_pointer_cast<Building>(objects[i])) {
            if (building->isInteractive()) {
                Vector3 doorPos = building->getDoorPosition();
                Vector3 toDoor = {
                    doorPos.x - camera.position.x,
                    doorPos.y - camera.position.y,
                    doorPos.z - camera.position.z
                };
                float distance = sqrtf(toDoor.x * toDoor.x + toDoor.y * toDoor.y + toDoor.z * toDoor.z);

                if (distance <= 3.0f && !state.isInBuilding) {
                    nearInteractable = true;
                    interactableName = "Press E to enter " + building->getName();
                    if (eKeyPressed) {
                        state.isInBuilding = true;
                        state.currentBuilding = static_cast<int>(i);
                        state.lastOutdoorPosition = camera.position;
                        camera.position = {building->position.x, 1.75f, building->position.z + 2.0f};
                        camera.target = {building->position.x, 1.55f, building->position.z + 5.0f};
                        state.playerY = 0.0f;
                        state.testBuildingEntry = true;
                        break;
                    }
                } else if (state.isInBuilding && state.currentBuilding == static_cast<int>(i) && eKeyPressed) {
                    state.isInBuilding = false;
                    camera.position = state.lastOutdoorPosition;
                    camera.target = {state.lastOutdoorPosition.x, state.lastOutdoorPosition.y - 0.2f, state.lastOutdoorPosition.z - 5.0f};
                    state.currentBuilding = -1;
                    break;
                }
            }
        }
    }

    for (int n = 0; n < MAX_NPCS; n++) {
        bool npcVisible = false;

        if (state.isInBuilding) {
            if (state.currentBuilding == 0 && n == 0) npcVisible = true;
            if (state.currentBuilding == 1 && n == 1) npcVisible = true;
        }

        if (npcVisible) {
            Vector3 toNPC = {
                npcs[n].position.x - camera.position.x,
                npcs[n].position.y - camera.position.y,
                npcs[n].position.z - camera.position.z
            };
            float distance = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y + toNPC.z * toNPC.z);

            if (distance <= npcs[n].interactionRadius * 1.5f) {
                nearInteractable = true;
                interactableName = "Press E to talk to " + npcs[n].name;
                if (eKeyPressed && !state.isInDialog) {
                    startDialog(n, state);
                    state.testNPCInteraction = true;
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