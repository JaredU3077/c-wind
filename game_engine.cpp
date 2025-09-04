// game_engine.cpp - Modern Game Engine Implementation
#include "game_engine.h"
#include "math_utils.h"
#include "debug_system.h"
#include "config.h"
#include "world_builder.h"
#include "combat.h"
#include "render_utils.h"
#include "ui_notification.h"
#include "ui_animation.h"
#include "ui_audio.h"
#include "npc.h"
#include "player_system.h"
#include <iostream>

// ===== MODERN GAME ENGINE IMPLEMENTATION =====
// Eliminates global variables through RAII and dependency injection

GameEngine::GameEngine() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "Initializing GameEngine");
    
    initializeSystems();
    initializeWorld();
    initializeCamera();
    
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "GameEngine initialization complete");
}

GameEngine::~GameEngine() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "GameEngine destructor called");
    shutdown();
}

void GameEngine::initializeSystems() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Initializing core systems");
    
    // Initialize systems with smart pointers (RAII)
    uiSystem_ = std::make_unique<UISystemManager>();
    inventory_ = std::make_unique<SimpleInventory>();
    inputManager_ = std::make_unique<InputManager>();
    environmentManager_ = std::make_unique<EnvironmentManager>();
    gameState_ = std::make_unique<GameState>();
    performanceStats_ = std::make_unique<AdvancedFrameStats>();
    
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Core systems initialized successfully");
}

void GameEngine::initializeWorld() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Initializing game world");
    
    // Initialize NPCs
    npcs_.resize(MAX_NPCS);
    // Note: NPC initialization will be handled in the main function for now
    
    // Build world using existing world builder
    // Note: World building will be handled in the main function for now
    
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Game world initialized successfully");
}

void GameEngine::initializeCamera() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Initializing camera system");
    
    camera_.position = {10.0f, 2.0f, 10.0f};
    camera_.target = {0.0f, 0.0f, 0.0f};
    camera_.up = {0.0f, 1.0f, 0.0f};
    camera_.fovy = 75.0f;
    camera_.projection = CAMERA_PERSPECTIVE;
    
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Camera system initialized successfully");
}

int GameEngine::run() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "Starting main game loop");
    
    bool shouldClose = false;
    int frameCounter = 0;
    NPCVisibilityState npcVisibility;
    
    while (!shouldClose) {
        // **PERFORMANCE PROFILING**: Start frame profiling
        DebugSystem::startFrameProfile();
        
        // Debug output every 10 seconds (reduced frequency)
        if (frameCounter % 600 == 0) {
            DEBUG_LOG(DEBUG_GENERAL, DEBUG_DETAILED, "Game loop iteration: " + std::to_string(frameCounter));
        }
        
        float deltaTime = GetFrameTime();
        frameCounter++;
        
        // **ADVANCED PERFORMANCE TRACKING**
        updateAdvancedFrameStats(*performanceStats_, deltaTime);
        
        // ===== PHASE 1: PROCESS INPUT =====
        processInput(shouldClose);
        
        // ===== PHASE 2: UPDATE GAME LOGIC =====
        updateGameLogic(deltaTime);
        
        // ===== PHASE 3: RENDER =====
        renderGame();
        
        // Performance profiling controls
        if (IsKeyPressed(KEY_F3)) {
            DebugSystem::generatePerformanceReport();
        }
        
        // Check for quit conditions from ESC menu
        if (gameState_->showEscMenu && gameState_->selectedMenuOption == 3 && IsKeyPressed(KEY_ENTER)) {
            shouldClose = true;
        }
    }
    
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "Main game loop ended. Total frames: " + std::to_string(frameCounter));
    return 0;
}

void GameEngine::processInput(bool& shouldClose) {
    DEBUG_LOG(DEBUG_INPUT, DEBUG_TRACE, "Processing input phase");
    
    // ESC menu system
    if (IsKeyPressed(KEY_ESCAPE)) {
        DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "ESC key pressed - Current states: Inventory=" +
                  std::to_string(gameState_->showInventoryWindow) + ", EscMenu=" +
                  std::to_string(gameState_->showEscMenu));

        if (gameState_->showInventoryWindow) {
            gameState_->showInventoryWindow = false;
            gameState_->enhancedInput.setMouseCaptured(true);
            gameState_->mouseReleased = false;
            DEBUG_LOG(DEBUG_INVENTORY, DEBUG_BASIC, "ESC - Inventory closed, returning to game");
        } else if (gameState_->showEscMenu) {
            gameState_->showEscMenu = false;
            gameState_->enhancedInput.setMouseCaptured(true);
            gameState_->mouseReleased = false;
        } else {
            gameState_->showEscMenu = true;
            gameState_->enhancedInput.setMouseCaptured(false);
            gameState_->mouseReleased = true;
            gameState_->selectedMenuOption = 0;
        }
    }

    // Window close handling
    static bool windowCloseRequested = false;
    if (WindowShouldClose() && !windowCloseRequested && !gameState_->showEscMenu) {
        windowCloseRequested = true;
        gameState_->showEscMenu = true;
        gameState_->enhancedInput.setMouseCaptured(false);
        gameState_->mouseReleased = true;
        gameState_->selectedMenuOption = 3; // Highlight "Quit Game"
    }
    
    if (!gameState_->showEscMenu) {
        windowCloseRequested = false;
    }

    // Inventory toggle
    if (!gameState_->isInDialog && !gameState_->showEscMenu && IsKeyPressed(KEY_I)) {
        gameState_->showInventoryWindow = !gameState_->showInventoryWindow;
        if (gameState_->showInventoryWindow) {
            gameState_->enhancedInput.setMouseCaptured(false);
            gameState_->mouseReleased = true;
            inventory_->show();
        } else {
            gameState_->enhancedInput.setMouseCaptured(true);
            gameState_->mouseReleased = false;
        }
    }

    // Testing panel toggle
    static bool lastTabState = false;
    bool currentTabState = gameState_->enhancedInput.isActionPressed("testing_panel");
    if (currentTabState && !lastTabState) {
        gameState_->showTestingPanel = !gameState_->showTestingPanel;
    }
    lastTabState = currentTabState;

    // Combat input
    if (!gameState_->isInDialog && !gameState_->showInventoryWindow && !gameState_->showEscMenu && 
        gameState_->enhancedInput.isMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        (GetTime() - gameState_->lastSwingTime) > gameState_->swingCooldown) {
        updateMeleeSwing(camera_, GetTime(), *gameState_);
    }

    // Input system updates
    inputManager_->update();
    gameState_->enhancedInput.update(GetFrameTime());
    
    // Inventory input handling (consolidated here)
    SAFE_CALL(
        inventory_->handleInput(),
        "inventory->handleInput()"
    );
}

void GameEngine::updateGameLogic(float deltaTime) {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_TRACE, "Updating game logic phase");
    
    // Update performance metrics
    gameState_->metrics.totalFrames++;
    gameState_->metrics.averageFrameTime = ((gameState_->metrics.averageFrameTime * (gameState_->metrics.totalFrames - 1)) + deltaTime) / gameState_->metrics.totalFrames;
    gameState_->metrics.frameTimeHistory.push_back(deltaTime);
    if (gameState_->metrics.frameTimeHistory.size() > 100) {
        gameState_->metrics.frameTimeHistory.erase(gameState_->metrics.frameTimeHistory.begin());
    }

    // Update UI systems
    UINotification::NotificationManager::getInstance().update(deltaTime);
    UIAnimation::AnimationManager::getInstance().update(deltaTime);
    UIAudio::AudioManager::getInstance().update(deltaTime);

    // Update environment and collision
    {
        ScopedTimer collisionTimer(performanceStats_->collisionTimer);
        environmentManager_->update(deltaTime, camera_);
    }

    // Update player system
    {
        ScopedTimer physicsTimer(performanceStats_->physicsTimer);
        updatePlayer(camera_, *gameState_, *environmentManager_, deltaTime);
    }

    // Handle building interactions
    auto interactiveObjects = environmentManager_->getInteractiveObjects();
    for (const auto& obj : interactiveObjects) {
        if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
            Vector3 doorPos = building->getDoorPosition();
            Vector3 toDoor = {
                doorPos.x - camera_.position.x,
                doorPos.y - camera_.position.y,
                doorPos.z - camera_.position.z
            };
            float distance = MathUtils::distance3D(doorPos, camera_.position);

            // Building entry/exit logic
            if (IsKeyPressed(KEY_E) && distance <= 3.0f) {
                if (!gameState_->isInBuilding) {
                    gameState_->isInBuilding = true;
                    gameState_->currentBuilding = building->getId();
                    gameState_->enhancedInput.setMouseCaptured(true);
                    gameState_->mouseReleased = false;
                } else {
                    gameState_->isInBuilding = false;
                    gameState_->currentBuilding = -1;
                    gameState_->enhancedInput.setMouseCaptured(true);
                    gameState_->mouseReleased = false;
                }
            }
        }
    }
}

void GameEngine::renderGame() {
    DEBUG_LOG(DEBUG_RENDERING, DEBUG_TRACE, "Starting render phase");
    
    performanceStats_->renderingTimer.start();
    
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera_);
            // Render world
            {
                ScopedTimer renderTimer(performanceStats_->renderingTimer);
                environmentManager_->renderAll(camera_);
            }
            
            // Render NPCs (visibility determined in update phase)
            NPCVisibilityState npcVisibility;
            for (int n = 0; n < MAX_NPCS; n++) {
                npcVisibility.shouldDraw[n] = false;
                if (gameState_->isInBuilding) {
                    if (gameState_->currentBuilding == 0 && n == 0) npcVisibility.shouldDraw[n] = true; // Mayor White
                    if (gameState_->currentBuilding == 1 && n == 1) npcVisibility.shouldDraw[n] = true; // Buster Shoppin
                }
                
                if (npcVisibility.shouldDraw[n]) {
                    renderNPC(npcs_[n], camera_, GetTime());
                }
            }

            // Render combat elements
            renderCombat(camera_, GetTime());

            // Render building interiors
            if (gameState_->isInBuilding && gameState_->currentBuilding >= 0) {
                auto objects = environmentManager_->getAllObjects();
                for (const auto& obj : objects) {
                    if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                        if (building->getId() == gameState_->currentBuilding) {
                            renderBuildingInterior(*building);
                            break;
                        }
                    }
                }
            }

            // Render interaction indicators (visual only)
            auto interactiveObjects = environmentManager_->getInteractiveObjects();
            for (const auto& obj : interactiveObjects) {
                if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                    Vector3 doorPos = building->getDoorPosition();
                    Vector3 toDoor = {
                        doorPos.x - camera_.position.x,
                        doorPos.y - camera_.position.y,
                        doorPos.z - camera_.position.z
                    };
                    float distance = MathUtils::distance3D(doorPos, camera_.position);

                    if (distance <= 3.0f && !gameState_->isInBuilding) {
                        float pulse = 0.7f + sinf(GetTime() * 5.0f) * 0.3f;
                        Vector3 indicatorPos = {doorPos.x, doorPos.y + 3.0f, doorPos.z};
                        DrawSphere(indicatorPos, 0.25f * pulse, YELLOW);
                        DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.3f));
                    } else if (distance <= 5.0f && !gameState_->isInBuilding) {
                        DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.1f));
                    }
                }
            }

        EndMode3D();

        // 2D UI rendering
        std::string locationText = "Town Square";
        Color locationColor = WHITE;
        if (gameState_->isInBuilding && gameState_->currentBuilding >= 0) {
            auto objects = environmentManager_->getAllObjects();
            for (const auto& obj : objects) {
                if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                    if (building->getId() == gameState_->currentBuilding) {
                        locationText = "Inside: " + building->getName();
                        break;
                    }
                }
            }
            locationColor = YELLOW;
        }
        DrawText(locationText.c_str(), 10, 40, 16, locationColor);

        // Jump indicator
        if (gameState_->isJumping) {
            float jumpHeight = gameState_->playerY - 0.0f;
            std::string jumpText = TextFormat("Jumping: %.1fm", jumpHeight);
            DrawText(jumpText.c_str(), 10, 160, 16, WHITE);
        }

        // UI systems rendering
        {
            ScopedTimer uiTimer(performanceStats_->uiTimer);
            SAFE_CALL(
                uiSystem_->renderAllUI(camera_, *gameState_, GetTime()),
                "uiSystem->renderAllUI()"
            );

            // Notifications
            SAFE_CALL(
                UINotification::NotificationManager::getInstance().render(),
                "UINotification::NotificationManager::render()"
            );
        }

        // Performance overlay
        renderAdvancedPerformanceOverlay(*performanceStats_, GetScreenWidth() - 310, 10);
        renderProjectedLabels(camera_, *environmentManager_, gameState_->isInBuilding, gameState_->currentBuilding);

        // Camera position debug info
        DrawText(TextFormat("Camera: %.1f, %.1f, %.1f", camera_.position.x, camera_.position.y, camera_.position.z),
                10, GetScreenHeight() - 30, 16, WHITE);

    EndDrawing();
    
    performanceStats_->renderingTimer.end();
    DebugSystem::endFrameProfile();
    
    // Performance profiling toggle (handled here where performanceStats is available)
    if (!gameState_->isInDialog && !gameState_->showEscMenu && !gameState_->showInventoryWindow && 
        gameState_->enhancedInput.isActionPressed("performance_toggle")) {
        performanceStats_->showDetailedStats = !performanceStats_->showDetailedStats;
        DEBUG_LOG(DEBUG_PERFORMANCE, DEBUG_BASIC, "Performance detailed stats: " + 
                 std::string(performanceStats_->showDetailedStats ? "ENABLED" : "DISABLED"));
    }
}

void GameEngine::shutdown() {
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "Shutting down GameEngine");
    
    // Release cursor
    if (gameState_) {
        gameState_->enhancedInput.setMouseCaptured(false);
    }
    
    if (inputManager_) {
        inputManager_->setMouseCaptured(false);
    }
    
    // Smart pointers automatically clean up
    // No manual delete calls needed!
    
    DEBUG_LOG(DEBUG_GENERAL, DEBUG_BASIC, "GameEngine shutdown complete");
}

void GameEngine::validatePerformance() const {
    if (performanceStats_) {
        bool isGood = isPerformanceGood(*performanceStats_);
        std::string report = getPerformanceReport(*performanceStats_);
        
        DEBUG_LOG(DEBUG_PERFORMANCE, DEBUG_BASIC, 
                 "Performance validation: " + std::string(isGood ? "GOOD" : "NEEDS_IMPROVEMENT"));
        DEBUG_LOG(DEBUG_PERFORMANCE, DEBUG_DETAILED, "Report: " + report);
    }
}
