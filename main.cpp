// main.cpp (updated with environment_manager enhancements)
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
#include "config.h"  
#include "errors.h"  
#include "input_manager.h"  

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

int main(void)
{
    std::cout << "Starting Browserwind main function..." << std::endl;

    // ===== MAC-SPECIFIC WINDOW FIXES =====
    // Get display size for proper window positioning
    int displayWidth = GetMonitorWidth(0);
    int displayHeight = GetMonitorHeight(0);
    std::cout << "Display size: " << displayWidth << "x" << displayHeight << std::endl;

    // Load configuration with fallback
    GameConfig config = loadConfig("config.ini");

    // Mac Retina display fix: Ensure window size doesn't exceed display
    if (config.windowWidth > displayWidth || config.windowHeight > displayHeight) {
        std::cout << "Window size too large for display, using 80% of screen size" << std::endl;
        config.windowWidth = (int)(displayWidth * 0.8f);
        config.windowHeight = (int)(displayHeight * 0.8f);
    }

    // Minimal size fallback
    if (config.windowWidth < 640 || config.windowHeight < 480) {
        config.windowWidth = 800;
        config.windowHeight = 600;
        std::cout << "Using minimal fallback size: 800x600" << std::endl;
    }

    std::cout << "Final window size: " << config.windowWidth << "x" << config.windowHeight << std::endl;

    try {
        // ===== WINDOW INITIALIZATION WITH MAC FIXES =====
        std::cout << "Initializing window with Mac-specific fixes..." << std::endl;

    // Set window flags for better Mac compatibility
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(config.windowWidth, config.windowHeight, config.windowTitle.c_str());
    
    // **CRITICAL FIX**: Disable raylib's default ESC-to-exit behavior
    SetExitKey(KEY_NULL);  // Prevent ESC from triggering WindowShouldClose()
    std::cout << "ESC key disabled from raylib - we handle it manually now" << std::endl;

    // Immediate window positioning and focus for Mac
    SetWindowPosition(100, 100);  // Position away from screen edges
    SetWindowState(FLAG_WINDOW_TOPMOST);  // Bring to front immediately

    // Verify window was created successfully
    if (!IsWindowReady()) {
        std::cerr << "ERROR: Window failed to initialize!" << std::endl;
        std::cerr << "Raylib version info should be available in console logs above" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Window initialized successfully" << std::endl;

    // Additional Mac-specific window management
    MaximizeWindow();  // Maximize to ensure visibility
    SetWindowState(FLAG_WINDOW_TOPMOST);  // Keep on top temporarily
    ClearWindowState(FLAG_WINDOW_TOPMOST);  // Then allow normal focus

    std::cout << "Window management complete - window should be visible now" << std::endl;

    // Give the window a moment to appear and gain focus
    std::cout << "Waiting 2 seconds for window to appear..." << std::endl;
    double waitStart = GetTime();
    while (GetTime() - waitStart < 2.0) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Browserwind Loading...", 300, 250, 30, WHITE);
        DrawText("Window should be visible now", 250, 290, 20, GREEN);
        DrawText("Press ESC to continue or wait...", 200, 330, 20, YELLOW);
        EndDrawing();

        if (IsKeyPressed(KEY_ESCAPE)) {
            std::cout << "ESC pressed during loading screen" << std::endl;
            break;
        }
    }

    std::cout << "Loading complete, proceeding with game..." << std::endl;

    // Skip fullscreen for debugging (can be re-enabled later)
    // if (config.fullscreen) {
    //     ToggleFullscreen();
    // }

            // Define the camera for first-person gameplay
        std::cout << "Setting up first-person camera..." << std::endl;
        Camera3D camera = {
            .position = (Vector3){ 0.0f, 1.75f, 5.0f },  // Start in front of the town square
            .target = (Vector3){ 0.0f, 1.75f, 0.0f },    // Look toward the center/well
            .up = (Vector3){ 0.0f, 1.0f, 0.0f },         // Standard up vector
            .fovy = 75.0f,                               // Slightly wider FOV for better visibility
            .projection = CAMERA_PERSPECTIVE              // First-person perspective
        };
        std::cout << "First-person camera setup complete" << std::endl;

        std::cout << "Setting up input manager..." << std::endl;
        InputManager input;  // Input manager
        input.setMouseCaptured(true);  // Capture mouse for first-person camera
        std::cout << "Input manager setup complete - mouse captured for FPS controls" << std::endl;

        std::cout << "Setting target FPS..." << std::endl;
        SetTargetFPS(config.targetFPS);  // Set our game to run at 60 frames-per-second
        std::cout << "Target FPS set to " << config.targetFPS << std::endl;

        // Central game state
        std::cout << "Initializing game state..." << std::endl;
        GameState state;
        state.lastCameraPos = camera.position;
        std::cout << "Game state initialized" << std::endl;
        
        // Initialize inventory system
        std::cout << "Initializing inventory system..." << std::endl;
        state.inventorySystem = std::make_unique<InventorySystem>(150.0f, 60);  // 150kg capacity, 60 slots
        state.inventorySystem->addStartingItems();  // Add basic starting gear
        std::cout << "Inventory system initialized with starting gear" << std::endl;

        // New: Add a state change listener (for demo, logs to console)
        std::cout << "Adding state change listener..." << std::endl;
        // Temporarily commented out for debugging
        // state.addChangeListener([](const std::string& property) {
        //     std::cout << "State changed: " << property << std::endl;
        // });
        std::cout << "State change listener skipped for debugging" << std::endl;

        // Skip state validation for debugging
        std::cout << "Skipping state validation for debugging..." << std::endl;
        // state.validateAndRepair();
        std::cout << "State validation skipped" << std::endl;

        // ===== PHASE 1: RE-ENABLE ENVIRONMENT MANAGER =====
        std::cout << "Re-enabling EnvironmentManager..." << std::endl;
        EnvironmentManager environment;
        std::cout << "EnvironmentManager created successfully" << std::endl;

        // Initialize world
        std::cout << "Initializing world..." << std::endl;
        initializeWorld(environment);
        std::cout << "World initialized successfully" << std::endl;

        // ===== PHASE 2: RE-ENABLE COMBAT SYSTEM =====
        std::cout << "Initializing combat system..." << std::endl;
        initCombat();
        std::cout << "Combat system initialized successfully" << std::endl;

        // ===== PHASE 3: RE-ENABLE NPC SYSTEM =====
        std::cout << "Initializing NPCs..." << std::endl;
        initNPCs();
        std::cout << "NPC system initialized successfully" << std::endl;

        std::cout << "All systems initialized successfully!" << std::endl;

        // Main game loop with custom ESC handling
        bool shouldClose = false;
        static float lastEscPressTime = -10.0f; // Static variable for double-ESC detection
        int frameCounter = 0;  // For periodic validation
        std::cout << "Starting main game loop..." << std::endl;

        // Debug: Test if window is ready
        std::cout << "Window ready check: " << IsWindowReady() << std::endl;
        std::cout << "Window should close check: " << WindowShouldClose() << std::endl;

        std::cout << "Starting main game loop..." << std::endl;
        while (!shouldClose)  // Custom close detection
        {
            float currentTime = GetTime();

            // Debug output every 60 frames
            if (frameCounter % 60 == 0) {
                std::cout << "Game loop iteration: " << frameCounter << ", Window ready: " << IsWindowReady() << std::endl;
            }
            float deltaTime = GetFrameTime();

            // New: Update performance metrics in state
            state.metrics.totalFrames++;
            state.metrics.averageFrameTime = ((state.metrics.averageFrameTime * (state.metrics.totalFrames - 1)) + deltaTime) / state.metrics.totalFrames;
            state.metrics.frameTimeHistory.push_back(deltaTime);
            if (state.metrics.frameTimeHistory.size() > 100) {
                state.metrics.frameTimeHistory.erase(state.metrics.frameTimeHistory.begin());
            }
            state.notifyChange("metrics");  // Notify on update

            // ===== PHASE 4: RE-ENABLE ENVIRONMENTAL UPDATES =====
            frameCounter++;
            environment.update(GetFrameTime(), camera);

            // ===== PHASE 5: CLEAN INPUT SYSTEM =====
            // **COMPREHENSIVE ESC MENU SYSTEM** - Handle all ESC scenarios properly
            if (IsKeyPressed(KEY_ESCAPE)) {
                std::cout << "DEBUG: ESC key detected! Current states - Inventory: " << state.showInventoryWindow 
                          << ", EscMenu: " << state.showEscMenu << std::endl;
                
                if (state.showInventoryWindow) {
                    // **CLOSE INVENTORY** if open, like pressing I again
                    state.showInventoryWindow = false;
                    input.setMouseCaptured(true);
                    state.mouseReleased = false;
                    std::cout << "ESC - Inventory closed, returning to game" << std::endl;
                } else if (state.showEscMenu) {
                    // **CLOSE ESC MENU** if open, resume game
                    state.showEscMenu = false;
                    input.setMouseCaptured(true);
                    state.mouseReleased = false;
                    std::cout << "ESC - Menu closed, resuming game" << std::endl;
                } else {
                    // **OPEN ESC MENU** - pause game and show options
                    state.showEscMenu = true;
                    input.setMouseCaptured(false);
                    state.mouseReleased = true;
                    state.selectedMenuOption = 0;  // Reset to "Resume"
                    std::cout << "ESC - Pause menu opened! Mouse freed for clicking." << std::endl;
                }
                
                // **CRITICAL**: Continue the loop - DO NOT EXIT!
                std::cout << "ESC handling complete - continuing game loop..." << std::endl;
            }

            // **WINDOW CLOSE CHECK** - Only X button or Alt+F4 (ESC disabled in raylib)
            if (WindowShouldClose()) {
                // Window X button clicked - show pause menu with Quit highlighted
                std::cout << "Window close requested - opening pause menu..." << std::endl;
                state.showEscMenu = true;
                input.setMouseCaptured(false);
                state.mouseReleased = true;
                state.selectedMenuOption = 3;  // Highlight "Quit Game" option
            }

            // NOTE: Camera controls are now handled by updatePlayer() function
            // which uses Raylib's proper CAMERA_FIRST_PERSON system

            // ===== PHASE 6: RE-ENABLE COMBAT MECHANICS =====
            // Update input manager first
            input.update();

            // Melee combat mechanics (longsword) - disabled during dialog or inventory
            if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu && input.isMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                (GetTime() - state.lastSwingTime) > state.swingCooldown) {
                updateMeleeSwing(camera, GetTime(), state);
                state.notifyChange("lastSwingTime");  // New: Example notify
            }
            
            // Inventory system controls - disabled during dialog or ESC menu
            if (!state.isInDialog && !state.showEscMenu && state.inventorySystem) {
                // Toggle inventory UI window with 'I' key - **AUTO-HANDLE MOUSE STATE**
                if (IsKeyPressed(KEY_I)) {
                    state.showInventoryWindow = !state.showInventoryWindow;
                    
                    if (state.showInventoryWindow) {
                        // **OPENING INVENTORY**: Auto-free mouse for UI interaction
                        input.setMouseCaptured(false);
                        state.mouseReleased = true;
                        std::cout << "Inventory opened - mouse automatically freed for interaction" << std::endl;
                    } else {
                        // **CLOSING INVENTORY**: Auto-capture mouse for gameplay
                        input.setMouseCaptured(true);
                        state.mouseReleased = false;
                        std::cout << "Inventory closed - mouse automatically captured for gameplay" << std::endl;
                    }
                }
                
                // **INVENTORY MOUSE INTERACTIONS** - Handle item clicks when mouse is free
                if (state.showInventoryWindow && !input.isMouseCaptured()) {
                    Vector2 mousePos = GetMousePosition();
                    int screenWidth = GetScreenWidth();
                    int screenHeight = GetScreenHeight();
                    int invWidth = 500;
                    int invHeight = 400;
                    int invX = (screenWidth - invWidth) / 2;
                    int invY = (screenHeight - invHeight) / 2;
                    
                    // Check if mouse is clicking in inventory area
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        // Item list area starts at invY + 85 (after title + headers)
                        int itemListY = invY + 85;
                        int itemHeight = 18;
                        
                        // Check each item for clicks
                        int itemIndex = (mousePos.y - itemListY) / itemHeight;
                        if (itemIndex >= 0 && itemIndex < 12 && mousePos.x >= invX + 20 && mousePos.x <= invX + 470) {
                            auto items = state.inventorySystem->getInventory().getAllItems();
                            if (itemIndex < (int)items.size()) {
                                auto clickedItem = items[itemIndex];
                                state.lastClickedItem = clickedItem->getName();
                                
                                std::cout << "Inventory Click: " << clickedItem->getName() << " [" 
                                         << ItemUtils::rarityToString(clickedItem->getRarity()) << "]" << std::endl;
                                
                                // **AUTO-EQUIP/USE LOGIC**
                                if (clickedItem->getType() == ItemType::WEAPON || clickedItem->getType() == ItemType::ARMOR) {
                                    // Try to equip the item
                                    if (state.inventorySystem->equipItemFromInventory(clickedItem->getName())) {
                                        std::cout << "Equipped: " << clickedItem->getName() << std::endl;
                                    } else {
                                        std::cout << "Cannot equip: " << clickedItem->getName() << std::endl;
                                    }
                                } else if (clickedItem->getType() == ItemType::CONSUMABLE) {
                                    // Use the item
                                    state.inventorySystem->getInventory().removeItem(clickedItem, 1);
                                    
                                    auto potion = std::dynamic_pointer_cast<AlchemicalPotion>(clickedItem);
                                    if (potion) {
                                        auto effects = potion->getEffects();
                                        state.playerHealth = std::min(state.maxPlayerHealth, state.playerHealth + effects.health);
                                        state.playerMana = std::min(state.maxPlayerMana, state.playerMana + effects.mana);
                                        state.playerStamina = std::min(state.maxPlayerStamina, state.playerStamina + effects.stamina);
                                        std::cout << "Used " << clickedItem->getName() << " - Applied effects: HP+" 
                                                 << effects.health << " MP+" << effects.mana << " SP+" << effects.stamina << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
                
                // Quick item usage with number keys (1-5 for consumables)
                if (IsKeyPressed(KEY_ONE)) {
                    auto consumables = state.inventorySystem->getInventory().findItemsByType(ItemType::CONSUMABLE);
                    if (!consumables.empty()) {
                        std::cout << "Used " << consumables[0]->getName() << std::endl;
                        state.inventorySystem->getInventory().removeItem(consumables[0], 1);
                        
                        // **Apply potion effects to player stats**
                        auto potion = std::dynamic_pointer_cast<AlchemicalPotion>(consumables[0]);
                        if (potion) {
                            auto effects = potion->getEffects();
                            state.playerHealth = std::min(state.maxPlayerHealth, state.playerHealth + effects.health);
                            state.playerMana = std::min(state.maxPlayerMana, state.playerMana + effects.mana);
                            state.playerStamina = std::min(state.maxPlayerStamina, state.playerStamina + effects.stamina);
                            std::cout << "Applied effects: HP+" << effects.health << " MP+" << effects.mana << " SP+" << effects.stamina << std::endl;
                        }
                    }
                }
            }
            
            // **ESC MENU INTERACTIONS** - Handle menu clicks when mouse is free
            if (state.showEscMenu && !input.isMouseCaptured()) {
                Vector2 mousePos = GetMousePosition();
                int screenWidth = GetScreenWidth();
                int screenHeight = GetScreenHeight();
                int menuWidth = 400;
                int menuHeight = 300;
                int menuX = (screenWidth - menuWidth) / 2;
                int menuY = (screenHeight - menuHeight) / 2;
                int optionY = menuY + 80;
                int optionSpacing = 50;
                
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // Check which menu option was clicked
                    for (int i = 0; i < 4; i++) {
                        int buttonY = optionY + i * optionSpacing - 5;
                        if (mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuWidth - 20 &&
                            mousePos.y >= buttonY && mousePos.y <= buttonY + 40) {
                            
                            state.selectedMenuOption = i;
                            std::cout << "ESC Menu clicked: Option " << i << std::endl;
                            
                            switch (i) {
                                case 0: // Resume Game
                                    state.showEscMenu = false;
                                    input.setMouseCaptured(true);
                                    state.mouseReleased = false;
                                    std::cout << "Resuming game..." << std::endl;
                                    break;
                                    
                                case 1: // Save Game
                                    std::cout << "Saving game..." << std::endl;
                                    state.saveState();
                                    std::cout << "Game saved successfully!" << std::endl;
                                    break;
                                    
                                case 2: // Load Game
                                    std::cout << "Loading game..." << std::endl;
                                    if (state.loadState()) {
                                        std::cout << "Game loaded successfully!" << std::endl;
                                    } else {
                                        std::cout << "No save file found or load failed!" << std::endl;
                                    }
                                    break;
                                    
                                case 3: // Quit Game
                                    std::cout << "Quitting game..." << std::endl;
                                    shouldClose = true;
                                    break;
                            }
                            break;
                        }
                    }
                }
            }

            // Update swings
            updateSwings();

            // Update targets (respawn after being hit)
            updateTargets();

            // Update player (jumping, movement, collisions) - **DISABLED DURING ESC MENU**
            if (!state.showEscMenu) {
                updatePlayer(camera, state, environment, GetFrameTime());
            }

            // ===== PHASE 8: RE-ENABLE INTERACTION SYSTEM =====
            // Handle interactions - disabled during dialog, inventory, or ESC menu
            if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu) {
                handleInteractions(camera, environment, state, GetTime());
            }

            // ===== PHASE 7: RE-ENABLE FULL RENDERING =====
            BeginDrawing();

                ClearBackground(RAYWHITE);

                BeginMode3D(camera);

                    // ===== RENDER GROUND =====
                    // Draw ground plane with dynamic color based on jump state
                    Color groundColor = LIGHTGRAY;
                    if (state.isJumping) {
                        groundColor = Fade(SKYBLUE, 0.8f); // Blue tint when jumping
                    } else if (!state.isGrounded) {
                        groundColor = Fade(YELLOW, 0.6f); // Yellow when falling
                    }
                    DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 16.0f, 16.0f }, groundColor);

                    // ===== RENDER ENVIRONMENTAL OBJECTS =====
                    environment.renderAll(camera);

                    // ===== DETECT BUILDING ENTRY (for NPCs to be visible) =====
                    // Check if player is inside any building based on position, not just door entry
                    bool playerInBuilding = false;
                    int buildingIndex = -1;

                    auto objects = environment.getAllObjects();
                    for (size_t i = 0; i < objects.size(); ++i) {
                        if (auto building = std::dynamic_pointer_cast<Building>(objects[i])) {
                            Vector3 bSize = building->getSize();
                            Vector3 bPos = building->position;

                            // Check if player is inside this building's bounds
                            if (camera.position.x >= bPos.x - bSize.x/2 && camera.position.x <= bPos.x + bSize.x/2 &&
                                camera.position.y >= bPos.y - bSize.y/2 && camera.position.y <= bPos.y + bSize.y/2 &&
                                camera.position.z >= bPos.z - bSize.z/2 && camera.position.z <= bPos.z + bSize.z/2) {
                                playerInBuilding = true;
                                buildingIndex = building->getId();  // Use building ID instead of array index
                                break;
                            }
                        }
                    }

                    // Update state based on position detection
                    if (playerInBuilding && !state.isInBuilding) {
                        state.isInBuilding = true;
                        state.currentBuilding = buildingIndex;
                        printf("Detected player inside building via position: %s\n",
                               objects[buildingIndex]->getName().c_str());
                    } else if (!playerInBuilding && state.isInBuilding) {
                        state.isInBuilding = false;
                        state.currentBuilding = -1;
                        printf("Player exited building\n");
                    }

                    // ===== RENDER NPCS =====
                    for (int n = 0; n < MAX_NPCS; n++) {
                        bool shouldDrawNPC = false;

                        // Determine if NPC should be visible based on current location
                        if (state.isInBuilding) {
                            // Inside building - only show NPCs that are inside the current building
                            if (state.currentBuilding == 0 && n == 0) shouldDrawNPC = true; // Mayor White in Mayor's Building (ID 0)
                            if (state.currentBuilding == 1 && n == 1) shouldDrawNPC = true; // Buster Shoppin in Shop Building (ID 1)
                        } else {
                            // Outside - NPCs are not visible from outside (they're inside buildings)
                            shouldDrawNPC = false;
                        }

                        if (shouldDrawNPC) {
                            renderNPC(npcs[n], camera, GetTime());
                        }
                    }

                    // ===== RENDER COMBAT ELEMENTS =====
                    renderCombat(camera, GetTime());

                    // ===== RENDER BUILDING INTERIORS =====
                    // Render building interiors when player is inside (AFTER exterior to avoid conflicts)
                    if (state.isInBuilding && state.currentBuilding >= 0) {
                        auto objects = environment.getAllObjects();
                        // Find building by ID instead of array index
                        for (const auto& obj : objects) {
                            if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                                if (building->getId() == state.currentBuilding) {
                                    // Interior rendering - no special depth settings needed with Raylib
                                    renderBuildingInterior(*building);
                                    break;
                                }
                            }
                        }
                    }

                    // ===== RENDER INTERACTION SYSTEM =====
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
                                float pulse = 0.7f + sinf(GetTime() * 5.0f) * 0.3f;
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

                    // ===== RENDER LOCATION INDICATOR =====
                    std::string locationText = "Town Square";
                    Color locationColor = WHITE;
                    if (state.isInBuilding && state.currentBuilding >= 0) {
                        auto objects = environment.getAllObjects();
                        // Find building by ID instead of array index
                        bool foundBuilding = false;
                        for (const auto& obj : objects) {
                            if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                                if (building->getId() == state.currentBuilding) {
                                    locationText = "Inside: " + building->getName();
                                    foundBuilding = true;
                                    break;
                                }
                            }
                        }
                        if (!foundBuilding) {
                            locationText = "Inside Building";
                        }
                        locationColor = YELLOW;
                    }

                    // Draw jump height indicator (optional visual feedback)
                    std::string jumpText = "";
                    if (state.isJumping) {
                        float jumpHeight = state.playerY - 0.0f;  // groundLevel
                        jumpText = TextFormat("Jumping: %.1fm", jumpHeight);
                    }

                EndMode3D();

                // ===== RENDER 2D TEXT (FIXED: No longer in 3D space!) =====
                // Location indicator - now properly in 2D
                DrawText(locationText.c_str(), 10, 40, 16, locationColor);

                // Jump indicator - now properly in 2D
                if (!jumpText.empty()) {
                    DrawText(jumpText.c_str(), 10, 160, 16, WHITE);
                }

                // ===== RENDER UI SYSTEMS =====
                // Draw UI elements
                renderUI(camera, GetTime(), state, state.testBuildingCollision);

                // **NEW: Render player stats** - Always visible in top-left
                renderPlayerStatsUI(state);

                // Render testing panel
                renderTestingPanel(state, locationText, locationColor);

                // Render game stats
                renderGameStats(state);

                // **NEW: Render inventory window** - Toggle with I key
                if (state.showInventoryWindow) {
                    renderInventoryUI(state);
                }
                
                // **NEW: Render ESC pause menu** - Toggle with ESC key (with hover detection)
                if (state.showEscMenu) {
                    renderEscMenu(state);  // **Updates selectedMenuOption** based on mouse hover
                }

                // Render dialog window
                if (state.isInDialog && state.showDialogWindow) {
                    renderDialogWindow(state);
                }

                // Render projected labels
                renderProjectedLabels(camera, environment, state.isInBuilding, state.currentBuilding);

                // ===== DIAGNOSTIC UI =====
                DrawRectangle(10, 10, 400, 160, DARKBLUE);
                DrawText("Browserwind - FULL GAME LOADED", 20, 15, 20, WHITE);
                DrawText(TextFormat("Frame: %d", frameCounter), 20, 40, 16, GREEN);
                DrawText(TextFormat("FPS: %d", GetFPS()), 20, 60, 16, GREEN);
                
                // **Enhanced mouse state display**
                Color mouseStateColor = input.isMouseCaptured() ? GREEN : YELLOW;
                const char* mouseStateText = input.isMouseCaptured() ? "CAPTURED (Gameplay)" : "FREE (UI Mode)";
                DrawText(TextFormat("Mouse: %s", mouseStateText), 20, 80, 16, mouseStateColor);
                
                // **Menu state display**
                if (state.showEscMenu) {
                    DrawText("Game: PAUSED - ESC Menu active", 20, 100, 16, RED);
                    DrawText("Controls: Click options | ESC=Resume", 20, 120, 14, RED);
                } else if (state.showInventoryWindow) {
                    DrawText("Inventory: OPEN - Click to interact!", 20, 100, 16, PURPLE);
                    DrawText("Controls: Click items | I=Close & return to game", 20, 120, 14, PURPLE);
                } else {
                    DrawText("Controls: WASD + Mouse + LMB | I=Inventory", 20, 100, 16, YELLOW);
                    DrawText("ESC: Pause menu | I=Inventory", 20, 120, 14, GREEN);
                }
                
                // **Clear instructions**
                DrawText("TIP: ESC opens pause menu (Save/Load/Quit)!", 20, 140, 12, ORANGE);

                // Show camera position
                DrawText(TextFormat("Camera: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z),
                        10, GetScreenHeight() - 30, 16, WHITE);

                // Render performance overlay (updated to use state.metrics)
                DrawText(TextFormat("Avg: %.2fms | Frames: %d",
                           state.metrics.averageFrameTime * 1000, state.metrics.totalFrames),
                         10, 10, 16, LIME);

            EndDrawing();
        }

        // Release cursor on exit
        input.setMouseCaptured(false);

        std::cout << "Game exited cleanly. Total frames: " << frameCounter << std::endl;

        // De-Initialization
        CloseWindow();  // Close window and OpenGL context

        return 0;

    } catch (const GameException& e) {
        std::cerr << "Game error (code " << static_cast<int>(e.getErrorCode()) << "): " << e.what() << std::endl;
        EnableCursor();
        CloseWindow();
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during game execution: " << e.what() << std::endl;
        std::cerr << "Exception type: " << typeid(e).name() << std::endl;
        EnableCursor();
        CloseWindow();
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred during game execution" << std::endl;
        EnableCursor();
        CloseWindow();
        return EXIT_FAILURE;
    }
}