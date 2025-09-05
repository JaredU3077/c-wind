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
#include "menu_system.h"  // For MenuSystem
#include "render_system.h"  // For RenderSystem

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>

void Entity::addComponent(std::unique_ptr<Component> component) {
    if (component) {
        components_[component->getTypeName()] = std::move(component);
    }
}

Component* Entity::getComponent(const std::string& typeName) const {
    auto it = components_.find(typeName);
    return (it != components_.end()) ? it->second.get() : nullptr;
}

Game::Game() {
    // Default constructor - initialization in Init()
}

Game::~Game() {
    Shutdown();
}

int Game::Run() {
    try {
        Init();

        while (!shouldClose_ && !state_.shouldClose) {
            float deltaTime = GetFrameTime();
            Update(deltaTime);
            // Re-enable render but with empty function to test
            Render();
            frameCounter_++;
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
    config_ = loadConfig("config.ini");

    // Mac Retina display fix: Ensure window size doesn't exceed display
    if (config_.windowWidth > displayWidth || config_.windowHeight > displayHeight) {
        std::cout << "Window size too large for display, using 80% of screen size" << std::endl;
        config_.windowWidth = static_cast<int>(displayWidth * 0.8f);
        config_.windowHeight = static_cast<int>(displayHeight * 0.8f);
    }

    // Minimal size fallback
    if (config_.windowWidth < 640 || config_.windowHeight < 480) {
        config_.windowWidth = 800;
        config_.windowHeight = 600;
        std::cout << "Using minimal fallback size: 800x600" << std::endl;
    }

    std::cout << "Final window size: " << config_.windowWidth << "x" << config_.windowHeight << std::endl;

    // ===== WINDOW INITIALIZATION WITH MAC FIXES =====
    std::cout << "Initializing window with Mac-specific fixes..." << std::endl;

    // Set window flags for better Mac compatibility
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(config_.windowWidth, config_.windowHeight, config_.windowTitle.c_str());

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

    // Give the window a moment to appear and focus
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
    SetTargetFPS(config_.targetFPS);
    std::cout << "Target FPS set to " << config_.targetFPS << std::endl;
}

void Game::InitSystems() {
    // Define the camera for first-person gameplay
    std::cout << "Setting up first-person camera..." << std::endl;
    camera_ = {
        .position = {0.0f, PlayerConstants::EYE_HEIGHT, 5.0f},  // Start in front of the town square
        .target = {0.0f, PlayerConstants::EYE_HEIGHT, 0.0f},    // Look toward the center/well
        .up = {0.0f, 1.0f, 0.0f},                              // Standard up vector
        .fovy = 75.0f,                                         // Slightly wider FOV for better visibility
        .projection = CAMERA_PERSPECTIVE                       // First-person perspective
    };
    std::cout << "First-person camera setup complete" << std::endl;

    std::cout << "Setting up input manager..." << std::endl;
    input_ = std::make_unique<InputManager>();
    input_->setMouseCaptured(true);  // Capture mouse for first-person camera
    enhancedInput_ = std::make_unique<EnhancedInputManager>();
    enhancedInput_->setMouseCaptured(true);  // Enhanced input
    std::cout << "Input manager setup complete - mouse captured for FPS controls" << std::endl;

    // Central game state
    std::cout << "Initializing game state..." << std::endl;
    state_.lastCameraPos = camera_.position;
    std::cout << "Game state initialized" << std::endl;

    // Initialize inventory system
    std::cout << "Initializing inventory system..." << std::endl;
    inventorySystem_ = std::make_unique<InventorySystem>(150.0f, 60);  // 150kg capacity, 60 slots
    inventorySystem_->addStartingItems();  // Add basic starting gear
    state_.inventorySystem = inventorySystem_.get();  // Raw pointer link (consider weak_ptr in future)
    std::cout << "Inventory system initialized with starting gear" << std::endl;

    // Initialize MenuSystem
    menuSystem_ = std::make_unique<MenuSystem>(state_);

    // ===== PHASE 1: CREATE ENVIRONMENT MANAGER =====
    std::cout << "Creating EnvironmentManager..." << std::endl;
    environment_ = std::make_unique<EnvironmentManager>();
    std::cout << "EnvironmentManager created successfully" << std::endl;

    // Initialize RenderSystem
    renderSystem_ = std::make_unique<RenderSystem>(state_, *environment_, performanceStats_);

    // Add a state change listener (for demo, logs to console)
    std::cout << "Adding state change listener..." << std::endl;
    state_.addChangeListener([](const std::string& property) {
        std::cout << "State changed: " << property << std::endl;
    });

    // Validate state
    state_.validateAndRepair();

    // **ENHANCED PERFORMANCE MONITORING** - Initialize advanced frame tracking
    std::cout << "Performance monitoring system initialized" << std::endl;

    // **UI SYSTEM**: Initialize organized UI management
    initializeUISystem();
    std::cout << "UI system initialized successfully" << std::endl;
}

void Game::InitWorldAndEntities() {
    // EnvironmentManager is now created in Init(), so we just initialize the world
    std::cout << "Initializing world..." << std::endl;
    initializeWorld(*environment_);
    std::cout << "World initialized successfully" << std::endl;

    // ===== PHASE 2: RE-ENABLE COMBAT SYSTEM =====
    std::cout << "Initializing combat system..." << std::endl;
    initCombat();
    std::cout << "Combat system initialized successfully" << std::endl;

    // ===== PHASE 3: RE-ENABLE NPC SYSTEM =====
    std::cout << "Initializing NPCs..." << std::endl;
    initNPCs();
    std::cout << "NPC system initialized successfully" << std::endl;

    // ECS migration start: Create sample entity (player placeholder)
    // ECS functionality temporarily disabled due to component conflicts
    // auto playerEntity = std::make_unique<Entity>();
    // auto transform = std::make_unique<TransformComponent>();
    // transform->position = camera_.position;
    // playerEntity->addComponent(std::move(transform));
    // entities_.push_back(std::move(playerEntity));
}

void Game::Update(float deltaTime) {
    // Debug output every 60 frames
    if (frameCounter_ % 60 == 0) {
        std::cout << "Game loop iteration: " << frameCounter_ << ", Window ready: " << IsWindowReady() << std::endl;
    }

    HandleInput(deltaTime);
    std::cout << "Finished HandleInput" << std::endl;

    UpdateSystems(deltaTime);
}

void Game::HandleInput(float deltaTime) {
    // **UPDATE INPUT MANAGER**
    // input_->update();  // Legacy - DISABLED to prevent mouse delta conflicts
    enhancedInput_->update(deltaTime);

    // **SYNC INPUT TO GAME STATE** - Critical for player system to get input
    state_.enhancedInput = *enhancedInput_;  // Copy the updated input state to game state

    // **NEW: REGISTER CALLBACKS ONCE (move to Init if static)**
    static bool callbacksRegistered = false;
    if (!callbacksRegistered) {
        std::cout << "Registering input callbacks..." << std::endl;
        enhancedInput_->registerActionCallback("pause", [&]() {
            std::cout << "DEBUG: ESC key detected! Current states - Inventory: " << state_.showInventoryWindow
                      << ", EscMenu: " << state_.showEscMenu << std::endl;

            if (state_.showInventoryWindow) {
                state_.showInventoryWindow = false;
                enhancedInput_->setMouseCaptured(true);
                state_.mouseReleased = false;
                std::cout << "ESC - Inventory closed, returning to game" << std::endl;
            } else if (state_.showEscMenu) {
                state_.showEscMenu = false;
                enhancedInput_->setMouseCaptured(true);
                state_.mouseReleased = false;
                std::cout << "ESC - Menu closed, resuming game" << std::endl;
            } else {
                state_.showEscMenu = true;
                enhancedInput_->setMouseCaptured(false);
                state_.mouseReleased = true;
                state_.selectedMenuOption = 0;
                std::cout << "ESC - Pause menu opened! Mouse freed for clicking." << std::endl;
            }
            state_.notifyChange("pause_menu");
        });

        enhancedInput_->registerActionCallback("testing_panel", [&]() {
            state_.showTestingPanel = !state_.showTestingPanel;
            std::cout << "TAB PRESSED: Testing panel: " << (state_.showTestingPanel ? "OPENED" : "CLOSED") << std::endl;
            state_.notifyChange("testing_panel");
        });

        enhancedInput_->registerActionCallback("performance_toggle", [&]() {
            if (!state_.isInDialog && !state_.showEscMenu && !state_.showInventoryWindow) {
                performanceStats_.showDetailedStats = !performanceStats_.showDetailedStats;
                std::cout << "Performance detailed stats: " << (performanceStats_.showDetailedStats ? "ENABLED" : "DISABLED") << std::endl;
            }
            state_.notifyChange("performance_toggle");
        });

        enhancedInput_->registerActionCallback("inventory", [&]() {
            if (!state_.isInDialog && !state_.showEscMenu) {
                state_.showInventoryWindow = !state_.showInventoryWindow;

                if (state_.showInventoryWindow) {
                    enhancedInput_->setMouseCaptured(false);
                    state_.mouseReleased = true;
                    std::cout << "Inventory opened - mouse automatically freed for interaction" << std::endl;
                } else {
                    enhancedInput_->setMouseCaptured(true);
                    state_.mouseReleased = false;
                    std::cout << "Inventory closed - mouse automatically captured for gameplay" << std::endl;
                }
            }
            state_.notifyChange("inventory");
        });

        enhancedInput_->registerActionCallback("quick_use", [&]() {
            if (!state_.isInDialog && !state_.showEscMenu && inventorySystem_) {
                auto consumables = inventorySystem_->getInventory().findItemsByType(ItemType::CONSUMABLE);
                if (!consumables.empty()) {
                    std::cout << "Used " << consumables[0]->getName() << std::endl;
                    inventorySystem_->getInventory().removeItem(consumables[0], 1);

                    auto potion = std::dynamic_pointer_cast<AlchemicalPotion>(consumables[0]);
                    if (potion) {
                        auto effects = potion->getEffects();
                        state_.playerHealth = std::min(state_.maxPlayerHealth, state_.playerHealth + effects.health);
                        state_.playerMana = std::min(state_.maxPlayerMana, state_.playerMana + effects.mana);
                        state_.playerStamina = std::min(state_.maxPlayerStamina, state_.playerStamina + effects.stamina);
                        std::cout << "Applied effects: HP+" << effects.health << " MP+" << effects.mana << " SP+" << effects.stamina << std::endl;
                    }
                }
            }
            state_.notifyChange("quick_use");
        });

        callbacksRegistered = true;
        std::cout << "Input callbacks registered successfully" << std::endl;
    }

    // **WINDOW CLOSE CHECK** - Keep inline for now (system-level)
    static bool windowCloseRequested = false;
    if (WindowShouldClose() && !windowCloseRequested && !state_.showEscMenu) {
        std::cout << "Window close requested - opening pause menu..." << std::endl;
        windowCloseRequested = true;
        state_.showEscMenu = true;
        enhancedInput_->setMouseCaptured(false);
        state_.mouseReleased = true;
        state_.selectedMenuOption = 3;
        state_.notifyChange("window_close");
    }
    if (!state_.showEscMenu) {
        windowCloseRequested = false;
    }

    // **MELEE SWING** - Move to combat if possible, but keep for now
    if (!state_.isInDialog && !state_.showInventoryWindow && !state_.showEscMenu &&
        enhancedInput_->isMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        (GetTime() - state_.lastSwingTime) > state_.swingCooldown) {
        updateMeleeSwing(camera_, GetTime(), state_);
        state_.notifyChange("melee_swing");
    }

    // **HANDLE MENU/INVENTORY INPUT** - Delegated to MenuSystem
    menuSystem_->handleEscMenuInput();
    menuSystem_->handleInventoryInput(*inventorySystem_);

    // **DEBUG**: RAW TAB check (keep for now)
    if (IsKeyPressed(KEY_TAB)) {
        std::cout << "RAW TAB KEY DETECTED!" << std::endl;
    }
}

void Game::UpdateSystems(float deltaTime) {
    // ===== PHASE 4: RE-ENABLE ENVIRONMENTAL UPDATES =====
    std::cout << "Starting environment update" << std::endl;
    if (environment_) {
        environment_->update(deltaTime, camera_);
    }
    std::cout << "Finished environment update" << std::endl;

    // NEW: Update building entry
    std::cout << "Starting updateBuildingEntry" << std::endl;
    updateBuildingEntry(camera_, state_, *environment_);
    std::cout << "Finished updateBuildingEntry" << std::endl;

    // Update swings
    std::cout << "Starting updateSwings" << std::endl;
    updateSwings();
    std::cout << "Finished updateSwings" << std::endl;

    // Update targets (respawn after being hit)
    std::cout << "Starting updateTargets" << std::endl;
    updateTargets();
    std::cout << "Finished updateTargets" << std::endl;

    // **PROFILED**: Update player (jumping, movement, collisions) - **DISABLED DURING ESC MENU**
    if (!state_.showEscMenu) {
        std::cout << "Starting updatePlayer" << std::endl;
        updatePlayer(camera_, state_, *environment_, deltaTime);
        std::cout << "Finished updatePlayer" << std::endl;
    }

    // ===== PHASE 8: RE-ENABLE INTERACTION SYSTEM =====
    // Handle interactions - disabled during dialog, inventory, or ESC menu
    if (!state_.isInDialog && !state_.showInventoryWindow && !state_.showEscMenu) {
        std::cout << "Starting handleInteractions" << std::endl;
        handleInteractions(camera_, *environment_, state_, GetTime());
        std::cout << "Finished handleInteractions" << std::endl;
    }

    // ECS update placeholder (for migration)
    std::cout << "Starting ECS update placeholder" << std::endl;
    for (auto& entity : entities_) {
        if (entity) {
            // Example: Update physics if present
            // if (auto physics = static_cast<PhysicsComponent*>(entity->getComponent("PhysicsComponent"))) {
            //     // Placeholder physics update
            //     physics->velocity.y -= 9.81f * deltaTime;  // Gravity example
            // }
        }
    }
    std::cout << "Finished ECS update placeholder" << std::endl;
}

void Game::Render() {
    // **PROFILED**: Start rendering performance tracking
    performanceStats_.renderingTimer.start();

    if (renderSystem_) {
        renderSystem_->renderAll(camera_, GetTime());
    }

    // **PROFILED**: Complete rendering performance tracking
    performanceStats_.renderingTimer.end();
}

void Game::Shutdown() {
    // **PHASE 2**: Release cursor on exit using enhanced input
    if (enhancedInput_) {
        enhancedInput_->setMouseCaptured(false);
    }
    if (input_) {
        input_->setMouseCaptured(false);  // Keep legacy for compatibility
    }

    // **UI SYSTEM**: Shutdown UI system
    shutdownUISystem();

    std::cout << "Game exited cleanly. Total frames: " << frameCounter_ << std::endl;

    // De-Initialization
    CloseWindow();  // Close window and OpenGL context
}