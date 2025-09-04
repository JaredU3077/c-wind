#include "game.h"
#include "environmental_object.h"
#include "collision_system.h"
#include "npc.h"
#include "dialog_system.h"
#include "combat.h"
#include "render_utils.h"
#include "interaction_system.h"
#include "player_system.h"
#include "world_builder.h"
#include "ui_system.h"  // For initializeUISystem() and shutdownUISystem()
#include "math_utils.h"  // For MathUtils
#include "constants.h"   // For constants like PlayerConstants, EnvironmentConstants
#include "menu_system.h"  // NEW
#include "render_system.h"  // NEW

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

Game::Game() {
    // Default constructor - initialization in Init()
}

Game::~Game() {
    Shutdown();
}

int Game::Run() {
    try {
        Init();

        while (!shouldClose && !state.shouldClose) {
            float deltaTime = GetFrameTime();
            Update(deltaTime);
            Render();
            frameCounter++;
        }

        Shutdown();
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during game execution: " << e.what() << std::endl;
        std::cerr << "Exception type: " << typeid(e).name() << std::endl;
        Shutdown();
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred during game execution" << std::endl;
        Shutdown();
        return EXIT_FAILURE;
    }
}

void Game::Init() {
    std::cout << "Starting Browserwind game initialization..." << std::endl;

    InitWindowAndConfig();
    InitSystems();
    InitWorldAndEntities();

    std::cout << "All systems initialized successfully!" << std::endl;
}

void Game::InitWindowAndConfig() {
    // ===== MAC-SPECIFIC WINDOW FIXES =====
    int displayWidth = GetMonitorWidth(0);
    int displayHeight = GetMonitorHeight(0);
    std::cout << "Display size: " << displayWidth << "x" << displayHeight << std::endl;

    // Load configuration with fallback
    config = loadConfig("config.ini");

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
        throw std::runtime_error("Window failed to initialize!");
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

    // Set target FPS
    SetTargetFPS(config.targetFPS);
    std::cout << "Target FPS set to " << config.targetFPS << std::endl;
}

void Game::InitSystems() {
    // Define the camera for first-person gameplay
    std::cout << "Setting up first-person camera..." << std::endl;
    camera = {
        .position = (Vector3){ 0.0f, PlayerConstants::EYE_HEIGHT, 5.0f },  // Start in front of the town square
        .target = (Vector3){ 0.0f, PlayerConstants::EYE_HEIGHT, 0.0f },    // Look toward the center/well
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },                              // Standard up vector
        .fovy = 75.0f,                                                    // Slightly wider FOV for better visibility
        .projection = CAMERA_PERSPECTIVE                                   // First-person perspective
    };
    std::cout << "First-person camera setup complete" << std::endl;

    std::cout << "Setting up input manager..." << std::endl;
    input.setMouseCaptured(true);  // Capture mouse for first-person camera
    enhancedInput.setMouseCaptured(true);  // Enhanced input
    state.enhancedInput.setMouseCaptured(true);  // Initialize state's enhanced input too

    // Initialize key bindings for state's input manager
    state.enhancedInput.setKeyBinding("move_forward", KEY_W);
    state.enhancedInput.setKeyBinding("move_backward", KEY_S);
    state.enhancedInput.setKeyBinding("strafe_left", KEY_A);
    state.enhancedInput.setKeyBinding("strafe_right", KEY_D);
    state.enhancedInput.setKeyBinding("jump", KEY_SPACE);
    state.enhancedInput.setKeyBinding("inventory", KEY_I);
    state.enhancedInput.setKeyBinding("interact", KEY_E);
    state.enhancedInput.setKeyBinding("pause", KEY_ESCAPE);
    state.enhancedInput.setKeyBinding("performance_toggle", KEY_P);
    state.enhancedInput.setKeyBinding("quick_use", KEY_ONE);
    state.enhancedInput.setKeyBinding("testing_panel", KEY_TAB);

    std::cout << "Input manager setup complete - mouse captured for FPS controls" << std::endl;

    // Central game state
    std::cout << "Initializing game state..." << std::endl;
    state.lastCameraPos = camera.position;
    std::cout << "Game state initialized" << std::endl;

    // Initialize inventory system
    std::cout << "Initializing inventory system..." << std::endl;
    inventorySystem = std::make_unique<InventorySystem>(150.0f, 60);  // 150kg capacity, 60 slots
    inventorySystem->addStartingItems();  // Add basic starting gear
    state.inventorySystem = inventorySystem.get();  // Link to state if needed; assume state uses pointer
    std::cout << "Inventory system initialized with starting gear" << std::endl;

    // NEW: Initialize MenuSystem
    menuSystem_ = std::make_unique<MenuSystem>(state);

    // NEW: Initialize RenderSystem
    renderSystem_ = std::make_unique<RenderSystem>(state, environment, performanceStats);

    // New: Add a state change listener (for demo, logs to console)
    std::cout << "Adding state change listener..." << std::endl;
    // state.addChangeListener([](const std::string& property) {
    //     std::cout << "State changed: " << property << std::endl;
    // });
    std::cout << "State change listener skipped for debugging" << std::endl;

    // Skip state validation for debugging
    std::cout << "Skipping state validation for debugging..." << std::endl;
    // state.validateAndRepair();
    std::cout << "State validation skipped" << std::endl;

    // **ENHANCED PERFORMANCE MONITORING** - Initialize advanced frame tracking
    std::cout << "Performance monitoring system initialized" << std::endl;

    // **UI SYSTEM**: Initialize organized UI management
    initializeUISystem();
    std::cout << "UI system initialized successfully" << std::endl;
}

void Game::InitWorldAndEntities() {
    // ===== PHASE 1: RE-ENABLE ENVIRONMENT MANAGER =====
    std::cout << "Re-enabling EnvironmentManager..." << std::endl;
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
}

void Game::Update(float deltaTime) {
    // Debug output every 60 frames
    if (frameCounter % 60 == 0) {
        std::cout << "Game loop iteration: " << frameCounter << ", Window ready: " << IsWindowReady() << std::endl;
    }

    // **PHASE 1 ENHANCEMENT**: Advanced performance tracking
    updateAdvancedFrameStats(performanceStats, deltaTime);

    // New: Update performance metrics in state
    state.metrics.totalFrames++;
    state.metrics.averageFrameTime = ((state.metrics.averageFrameTime * (state.metrics.totalFrames - 1)) + deltaTime) / state.metrics.totalFrames;

    state.metrics.frameTimeHistory.push_back(deltaTime);
    if (state.metrics.frameTimeHistory.size() > 100) {
        state.metrics.frameTimeHistory.erase(state.metrics.frameTimeHistory.begin());
    }
    // **TEMP DEBUG**: Disable all state change notifications to prevent crashes
    // state.notifyChange("metrics");  // Notify on update

    HandleInput(deltaTime);
    UpdateSystems(deltaTime);
}

void Game::HandleInput(float deltaTime) {
    // **UPDATE INPUT MANAGER**
    {
        PROFILE_SYSTEM(performanceStats, input);
        input.update();  // Legacy
        enhancedInput.update(deltaTime);
        state.enhancedInput.update(deltaTime);  // Update state's input manager too
    }

    // **NEW: REGISTER CALLBACKS ONCE (move to Init if static)**
    static bool callbacksRegistered = false;
    if (!callbacksRegistered) {
        enhancedInput.registerActionCallback("pause", [&]() {
            std::cout << "DEBUG: ESC key detected! Current states - Inventory: " << state.showInventoryWindow
                      << ", EscMenu: " << state.showEscMenu << std::endl;

            if (state.showInventoryWindow) {
                state.showInventoryWindow = false;
                enhancedInput.setMouseCaptured(true);
                state.mouseReleased = false;
                std::cout << "ESC - Inventory closed, returning to game" << std::endl;
            } else if (state.showEscMenu) {
                state.showEscMenu = false;
                enhancedInput.setMouseCaptured(true);
                state.mouseReleased = false;
                std::cout << "ESC - Menu closed, resuming game" << std::endl;
            } else {
                state.showEscMenu = true;
                enhancedInput.setMouseCaptured(false);
                state.mouseReleased = true;
                state.selectedMenuOption = 0;
                std::cout << "ESC - Pause menu opened! Mouse freed for clicking." << std::endl;
            }
        });

        enhancedInput.registerActionCallback("testing_panel", [&]() {
            state.showTestingPanel = !state.showTestingPanel;
            std::cout << "TAB PRESSED: Testing panel: " << (state.showTestingPanel ? "OPENED" : "CLOSED") << std::endl;
        });

        enhancedInput.registerActionCallback("performance_toggle", [&]() {
            if (!state.isInDialog && !state.showEscMenu && !state.showInventoryWindow) {
                performanceStats.showDetailedStats = !performanceStats.showDetailedStats;
                std::cout << "Performance detailed stats: " << (performanceStats.showDetailedStats ? "ENABLED" : "DISABLED") << std::endl;
            }
        });

        enhancedInput.registerActionCallback("inventory", [&]() {
            if (!state.isInDialog && !state.showEscMenu) {
                state.showInventoryWindow = !state.showInventoryWindow;

                if (state.showInventoryWindow) {
                    enhancedInput.setMouseCaptured(false);
                    state.mouseReleased = true;
                    std::cout << "Inventory opened - mouse automatically freed for interaction" << std::endl;
                } else {
                    enhancedInput.setMouseCaptured(true);
                    state.mouseReleased = false;
                    std::cout << "Inventory closed - mouse automatically captured for gameplay" << std::endl;
                }
            }
        });

        enhancedInput.registerActionCallback("quick_use", [&]() {
            if (!state.isInDialog && !state.showEscMenu && inventorySystem) {
                auto consumables = inventorySystem->getInventory().findItemsByType(ItemType::CONSUMABLE);
                if (!consumables.empty()) {
                    std::cout << "Used " << consumables[0]->getName() << std::endl;
                    inventorySystem->getInventory().removeItem(consumables[0], 1);

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
        });

        callbacksRegistered = true;
    }

    // **WINDOW CLOSE CHECK** - Keep inline for now (system-level)
    static bool windowCloseRequested = false;
    if (WindowShouldClose() && !windowCloseRequested && !state.showEscMenu) {
        std::cout << "Window close requested - opening pause menu..." << std::endl;
        windowCloseRequested = true;
        state.showEscMenu = true;
        enhancedInput.setMouseCaptured(false);
        state.mouseReleased = true;
        state.selectedMenuOption = 3;
    }
    if (!state.showEscMenu) {
        windowCloseRequested = false;
    }

    // **MELEE SWING** - Move to combat if possible, but keep for now
    if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu &&
        enhancedInput.isMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        (GetTime() - state.lastSwingTime) > state.swingCooldown) {
        updateMeleeSwing(camera, GetTime(), state);
    }

    // **HANDLE MENU/INVENTORY INPUT** - Delegated to MenuSystem
    menuSystem_->handleEscMenuInput();
    menuSystem_->handleInventoryInput(inventorySystem);

    // **DEBUG**: RAW TAB check (keep for now)
    if (IsKeyPressed(KEY_TAB)) {
        std::cout << "RAW TAB KEY DETECTED!" << std::endl;
    }
}

void Game::UpdateSystems(float deltaTime) {
    // ===== PHASE 4: RE-ENABLE ENVIRONMENTAL UPDATES =====
    // **PROFILED**: Environment and collision updates
    {
        PROFILE_SYSTEM(performanceStats, collision);
        environment.update(deltaTime, camera);
    }

    // NEW: Update building entry
    updateBuildingEntry(camera, state, environment);

    // Update swings
    updateSwings();

    // Update targets (respawn after being hit)
    updateTargets();

    // **PROFILED**: Update player (jumping, movement, collisions) - **DISABLED DURING ESC MENU**
    if (!state.showEscMenu) {
        PROFILE_SYSTEM(performanceStats, physics);
        updatePlayer(camera, state, environment, deltaTime);
    } else {
        // Debug: Log when player movement is disabled due to ESC menu
        static int escMenuDebugCounter = 0;
        if (escMenuDebugCounter++ % 60 == 0) {  // Log every second at 60 FPS
            std::cout << "PLAYER MOVEMENT DISABLED: ESC Menu is active (showEscMenu=" << state.showEscMenu << ")" << std::endl;
        }
    }

    // ===== PHASE 8: RE-ENABLE INTERACTION SYSTEM =====
    // Handle interactions - disabled during dialog, inventory, or ESC menu
    if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu) {
        handleInteractions(camera, environment, state, GetTime());
    }
}

void Game::Render() {
    // **PROFILED**: Start rendering performance tracking
    performanceStats.renderingTimer.start();

    renderSystem_->renderAll(camera, GetTime());

    // **PROFILED**: Complete rendering performance tracking
    performanceStats.renderingTimer.end();
}

void Game::Shutdown() {
    // **PHASE 2**: Release cursor on exit using enhanced input
    enhancedInput.setMouseCaptured(false);
    input.setMouseCaptured(false);  // Keep legacy for compatibility

    // **UI SYSTEM**: Shutdown UI system
    shutdownUISystem();

    std::cout << "Game exited cleanly. Total frames: " << frameCounter << std::endl;

    // De-Initialization
    CloseWindow();  // Close window and OpenGL context
}