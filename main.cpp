// main.cpp (updated)
#include "raylib.h"
#include "environmental_object.h"
#include "collision_system.h"
#include "environment_manager.h"
#include "npc.h"
#include "dialog_system.h"
#include "combat.h"
#include "render_utils.h"
#include "interaction_system.h"
#include "player_system.h"
#include "testing_system.h"
#include "world_builder.h"
#include "game_state.h"

#include <iostream>
#include <vector>
#include <memory>
#include <string>

int main(void)
{
    // Initialization
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Browserwind - 3D FPS Game (1920x1080)");

    // Define the camera
    Camera3D camera = {
        .position = (Vector3){ 4.0f, 1.75f, 4.0f },   // Camera starting position (eye height)
        .target = (Vector3){ 0.0f, 1.55f, 0.0f },     // Camera looking at point
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },         // Camera up vector (rotation towards target)
        .fovy = 60.0f,                               // Camera field-of-view Y
        .projection = CAMERA_PERSPECTIVE              // Camera projection type
    };

    // Hide and capture mouse cursor for FPS gameplay
    DisableCursor(); // Mouse starts captured

    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second

    // Central game state
    GameState state;
    state.lastCameraPos = camera.position;

    // ===== NEW ENVIRONMENTAL OBJECT SYSTEM =====
    EnvironmentManager environment;

    // Initialize world
    initializeWorld(environment);

    // Initialize combat system
    initCombat();

    // Initialize NPCs
    initNPCs();

    // Main game loop with custom ESC handling
    float lastEscPressTime = -10.0f; // Initialize to a time long ago
    bool shouldClose = false;
    while (!shouldClose)  // Custom close detection
    {
        float currentTime = GetTime();
        float deltaTime = GetFrameTime();

        // Handle ESC key for mouse capture/release
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (currentTime - lastEscPressTime < 0.5f) {
                // Double ESC press within 0.5 seconds: Close window
                shouldClose = true;
                break;
            } else {
                // Single ESC press: Toggle mouse capture
                if (!state.mouseReleased) {
                    EnableCursor();
                    state.mouseReleased = true;
                } else {
                    DisableCursor();
                    state.mouseReleased = false;
                }
                lastEscPressTime = currentTime;
            }
        }

        // Check for window close button (X button or Alt+F4)
        if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4)) {
            shouldClose = true;
            break;
        }

        // Only check WindowShouldClose() if ESC wasn't pressed recently
        if (WindowShouldClose() && (currentTime - lastEscPressTime) > 1.0f) {
            shouldClose = true;
            break;
        }

        // Handle testing updates
        handleTesting(camera, environment, state);

        // Handle interactions
        handleInteractions(camera, environment, state, currentTime);

        // Melee combat mechanics (longsword) - disabled during dialog
        if (!state.isInDialog && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (currentTime - lastSwingTime) > swingCooldown) {
            updateMeleeSwing(camera, currentTime, state);
        }

        // Update swings
        updateSwings();

        // Update targets (respawn after being hit)
        updateTargets();

        // Update player (jumping, movement, collisions)
        updatePlayer(camera, state, environment, deltaTime);

        // Draw
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw ground plane with dynamic color based on jump state
                Color groundColor = LIGHTGRAY;
                if (state.isJumping) {
                    groundColor = Fade(SKYBLUE, 0.8f); // Blue tint when jumping
                } else if (!state.isGrounded) {
                    groundColor = Fade(YELLOW, 0.6f); // Yellow when falling
                }
                DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 16.0f, 16.0f }, groundColor);

                // ===== RENDER ENVIRONMENTAL OBJECTS =====
                environment.renderAll();

                // ===== BUILDING INTERIOR RENDERING =====
                // Render building interiors when player is inside
                if (state.isInBuilding && state.currentBuilding >= 0) {
                    auto objects = environment.getAllObjects();
                    if (state.currentBuilding < static_cast<int>(objects.size())) {
                        if (auto building = std::dynamic_pointer_cast<Building>(objects[state.currentBuilding])) {
                            renderBuildingInterior(*building);
                        }
                    }
                }

                // ===== INTERACTION SYSTEM =====
                // Handle building interactions
                auto interactiveObjects = environment.getInteractiveObjects();
                for (const auto& obj : interactiveObjects) {
                    if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                        Vector3 doorPos = building->getDoorPosition();
                        Vector3 toDoor = {
                            doorPos.x - camera.position.x,
                            doorPos.y - camera.position.y,
                            doorPos.z - camera.position.z
                        };
                        float distance = sqrtf(toDoor.x * toDoor.x + toDoor.y * toDoor.y + toDoor.z * toDoor.z);

                        if (distance <= 3.0f && !state.isInBuilding) {
                            // Bright, pulsing interaction indicator
                            float pulse = 0.7f + sinf(currentTime * 5.0f) * 0.3f;
                            Vector3 indicatorPos = {doorPos.x, doorPos.y + 3.0f, doorPos.z};
                            DrawSphere(indicatorPos, 0.25f * pulse, YELLOW);

                            // Enhanced interaction range visualization
                            DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.3f));
                        } else if (distance <= 5.0f && !state.isInBuilding) {
                            // Subtle approach indicator
                            DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.1f));
                        }
                    }
                }

                // Draw NPCs (only show NPCs that are in the current area)
                for (int n = 0; n < MAX_NPCS; n++) {
                    bool shouldDrawNPC = false;

                    // Determine if NPC should be visible based on current location
                    if (state.isInBuilding) {
                        // Inside building - only show NPCs that are inside the current building
                        if (state.currentBuilding == 0 && n == 0) shouldDrawNPC = true; // Mayor White in Mayor's Building
                        if (state.currentBuilding == 1 && n == 1) shouldDrawNPC = true; // Buster Shoppin in Shop Building
                    } else {
                        // Outside - NPCs are not visible from outside (they're inside buildings)
                        shouldDrawNPC = false;
                    }

                    if (shouldDrawNPC) {
                        renderNPC(npcs[n], camera, currentTime);
                    }
                }

                // Current location indicator
                std::string locationText = "Town Square";
                Color locationColor = GREEN;
                if (state.isInBuilding && state.currentBuilding >= 0) {
                    auto objects = environment.getAllObjects();
                    if (state.currentBuilding < objects.size()) {
                        locationText = "Inside: " + objects[state.currentBuilding]->getName();
                    } else {
                        locationText = "Inside Building";
                    }
                    locationColor = YELLOW;
                }
                DrawText(locationText.c_str(), 10, 40, 16, locationColor);

                // Render combat visuals
                renderCombat(camera, currentTime);

                // Draw jump height indicator (optional visual feedback)
                if (state.isJumping) {
                    float jumpHeight = state.playerY - 0.0f;  // groundLevel
                    DrawText(TextFormat("Jumping: %.1fm", jumpHeight), 10, 160, 16, WHITE);
                }

            EndMode3D();

            // Draw UI elements
            renderUI(camera, currentTime, state, state.testBuildingCollision);

            // Render testing panel
            renderTestingPanel(state, locationText, locationColor);

            // Render game stats
            renderGameStats(state);

            // Render dialog window
            if (state.isInDialog && state.showDialogWindow) {
                renderDialogWindow(state);
            }

            // Render projected labels
            renderProjectedLabels(camera, environment, state.isInBuilding, state.currentBuilding);

        EndDrawing();
    }

    // Keep cursor captured during gameplay, only release on exit
    // Note: We don't need to re-enable cursor during dialog since it should stay captured
    EnableCursor();

    // Final test summary
    printFinalSummary(state);

    // De-Initialization
    CloseWindow();  // Close window and OpenGL context

    return 0;
}