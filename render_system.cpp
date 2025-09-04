#include "render_system.h"
#include "math_utils.h"
#include "ui_system.h"  // For g_uiSystem
#include <iostream>

extern NPC npcs[MAX_NPCS];  // From npc.cpp

RenderSystem::RenderSystem(GameState& state, EnvironmentManager& environment, AdvancedFrameStats& performanceStats)
    : state_(state), environment_(environment), performanceStats_(performanceStats) {}

void RenderSystem::renderAll(const Camera3D& camera, float time) {
    BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            render3DWorld(camera, time);
        EndMode3D();

        render2DOverlays(camera, time);
    EndDrawing();
}

void RenderSystem::render3DWorld(const Camera3D& camera, float time) {
    // Render ground
    Color groundColor = LIGHTGRAY;
    if (state_.isJumping) {
        groundColor = Fade(SKYBLUE, 0.8f);
    } else if (!state_.isGrounded) {
        groundColor = Fade(YELLOW, 0.6f);
    }
    DrawPlane({0.0f, 0.0f, 0.0f}, {16.0f, 16.0f}, groundColor);

    // Render environment
    environment_.renderAll(camera);

    // Render NPCs
    for (int n = 0; n < MAX_NPCS; n++) {
        bool shouldDrawNPC = false;
        if (state_.isInBuilding) {
            if (state_.currentBuilding == 0 && n == 0) shouldDrawNPC = true;
            if (state_.currentBuilding == 1 && n == 1) shouldDrawNPC = true;
        }
        if (shouldDrawNPC) {
            renderNPC(npcs[n], camera, time);
        }
    }

    // Render combat
    renderCombat(camera, time);

    // Render building interiors
    if (state_.isInBuilding && state_.currentBuilding >= 0) {
        auto objects = environment_.getAllObjects();
        for (const auto& obj : objects) {
            if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                if (building->getId() == state_.currentBuilding) {
                    renderBuildingInterior(*building);
                    break;
                }
            }
        }
    }

    // Render interactions
    render3DInteractions(camera);
}

void RenderSystem::render3DInteractions(const Camera3D& camera) {
    auto interactiveObjects = environment_.getInteractiveObjects();
    for (const auto& obj : interactiveObjects) {
        if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
            Vector3 doorPos = building->getDoorPosition();
            float distance = MathUtils::distance3D(doorPos, camera.position);

            if (distance <= EnvironmentConstants::INTERACTION_DISTANCE && !state_.isInBuilding) {
                float pulse = 0.7f + sinf(GetTime() * 5.0f) * 0.3f;
                Vector3 indicatorPos = {doorPos.x, doorPos.y + 3.0f, doorPos.z};
                DrawSphere(indicatorPos, 0.25f * pulse, YELLOW);
                DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.3f));
            } else if (distance <= 5.0f && !state_.isInBuilding) {
                DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.1f));
            }
        }
    }
}

void RenderSystem::render2DOverlays(const Camera3D& camera, float time) {
    // Location text
    std::string locationText = getLocationText();
    Color locationColor = (state_.isInBuilding) ? YELLOW : WHITE;
    DrawText(locationText.c_str(), 10, 40, 16, locationColor);

    // Jump text
    std::string jumpText = getJumpText();
    if (!jumpText.empty()) {
        DrawText(jumpText.c_str(), 10, 160, 16, WHITE);
    }

    // UI system
    {
        PROFILE_SYSTEM(performanceStats_, ui);
        g_uiSystem->renderAllUI(camera, state_, time);
    }

    // Performance overlay
    renderAdvancedPerformanceOverlay(performanceStats_, GetScreenWidth() - 310, 10);

    // Projected labels
    renderProjectedLabels(camera, environment_, state_.isInBuilding, state_.currentBuilding);

    // Debug camera pos
    DrawText(TextFormat("Camera: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z),
             10, GetScreenHeight() - 30, 16, WHITE);
}

std::string RenderSystem::getLocationText() const {
    std::string locationText = "Town Square";
    if (state_.isInBuilding && state_.currentBuilding >= 0) {
        auto objects = environment_.getAllObjects();
        bool foundBuilding = false;
        for (const auto& obj : objects) {
            if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                if (building->getId() == state_.currentBuilding) {
                    locationText = "Inside: " + building->getName();
                    foundBuilding = true;
                    break;
                }
            }
        }
        if (!foundBuilding) {
            locationText = "Inside Building";
        }
    }
    return locationText;
}

std::string RenderSystem::getJumpText() const {
    if (state_.isJumping) {
        float jumpHeight = state_.playerY - 0.0f;  // groundLevel
        return TextFormat("Jumping: %.1fm", jumpHeight);
    }
    return "";
}