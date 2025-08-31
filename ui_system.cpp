// ui_system.cpp
#include "ui_system.h"
#include "render_utils.h"
#include "performance.h"
#include <iostream>
#include <cmath>

// Global UI system instance
UISystemManager* g_uiSystem = nullptr;

void initializeUISystem() {
    if (!g_uiSystem) {
        g_uiSystem = new UISystemManager();
    }
}

void shutdownUISystem() {
    if (g_uiSystem) {
        delete g_uiSystem;
        g_uiSystem = nullptr;
    }
}

UISystemManager::UISystemManager() : screenWidth_(800), screenHeight_(600) {
    initializeLayout();
}

UISystemManager::~UISystemManager() {
    // Cleanup any resources
}

void UISystemManager::setScreenDimensions(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    initializeLayout();
}

void UISystemManager::initializeLayout() {
    // Clear existing zone reservations
    zoneReservations_.clear();
}

Rectangle UISystemManager::getZoneBounds(UIZone zone) const {
    switch (zone) {
        case UIZone::TOP_LEFT:
            return {10, 10, 250, 220};  // Increased width and height for better spacing
        case UIZone::TOP_RIGHT:
            return {(float)(screenWidth_ - 300), 10, 290, 140};  // Increased height to prevent overlap between performance and testing panels
        case UIZone::CENTER:
            return {(float)(screenWidth_ / 2 - 320), (float)(screenHeight_ / 2 - 220), 640, 440};  // Modal windows
        case UIZone::BOTTOM_LEFT:
            return {10, (float)(screenHeight_ - 180), 380, 170};  // Increased space for controls and stats
        case UIZone::BOTTOM_RIGHT:
            return {(float)(screenWidth_ - 280), (float)(screenHeight_ - 120), 270, 110};
        case UIZone::INTERACTION:
            return {(float)(screenWidth_ / 2 - 175), (float)(screenHeight_ / 2 - 25), 350, 50};  // Centered interaction prompts
        default:
            return {0, 0, 100, 100};
    }
}

bool UISystemManager::isModalActive(const GameState& state) const {
    return state.isInDialog || state.showInventoryWindow || state.showEscMenu;
}

void UISystemManager::renderAllUI(Camera3D camera, const GameState& state, float currentTime) {
    // Update screen dimensions
    setScreenDimensions(GetScreenWidth(), GetScreenHeight());

    // Clear previous zone reservations
    clearOverlaps();

    // Render in layer order (background to foreground)
    renderBackgroundLayer(state);

    if (!isModalActive(state)) {
        renderGameplayLayer(camera, state, currentTime);
    }

    renderModalLayer(state);
    renderOverlayLayer(const_cast<GameState&>(state));  // ESC menu needs to update selection
    renderDebugLayer(state, currentTime);
}

void UISystemManager::renderBackgroundLayer(const GameState& state) {
    // Always-visible core UI elements
    renderPlayerStats(state);
    renderPerformanceDisplay(state);
}

void UISystemManager::renderGameplayLayer(Camera3D camera, const GameState& state, float currentTime) {
    // Only render when NOT in modal mode
    renderCrosshair(state);
    renderInteractionPrompt(state, currentTime);
    renderControlsPanel(state);
    renderGameStats(state);
}

void UISystemManager::renderModalLayer(const GameState& state) {
    // Modal dialogs - render one at a time, centered
    if (state.isInDialog && state.showDialogWindow) {
        renderDialogModal(state);
    } else if (state.showInventoryWindow) {
        renderInventoryModal(state);
    }
}

void UISystemManager::renderOverlayLayer(GameState& state) {
    // Full-screen overlays
    if (state.showEscMenu) {
        renderEscMenuOverlay(state);
    }
}

void UISystemManager::renderDebugLayer(const GameState& state, float currentTime) {
    // Debug/diagnostic info - always on top but minimized
    renderDiagnosticPanel(state);
    
    // **TAB TOGGLE**: Show detailed testing panel when requested
    if (state.showTestingPanel) {
        // Calculate location info for testing panel
        std::string locationText = state.isInBuilding ? "INSIDE BUILDING" : "OUTSIDE";
        Color locationColor = state.isInBuilding ? GREEN : BLUE;
        renderDetailedTestingPanel(state, locationText, locationColor);
    } else {
        // Show compact testing status in top-right
        renderTestingStatus(state);
    }
}

// ===== INDIVIDUAL COMPONENT RENDERERS - PROPERLY POSITIONED =====

void UISystemManager::renderCrosshair(const GameState& state) {
    if (state.isInDialog || state.mouseReleased) return;
    
    int centerX = screenWidth_ / 2;
    int centerY = screenHeight_ / 2;
    DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
    DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
}

void UISystemManager::renderPlayerStats(const GameState& state) {
    // **TOP-LEFT ZONE** - Properly positioned below diagnostic panels
    Rectangle zone = calculateNonOverlappingPosition(UIZone::TOP_LEFT, 230, 130);
    int statsX = (int)zone.x + 5;
    int statsY = (int)zone.y + 85;  // Positioned below diagnostic panels with proper spacing
    int statsWidth = 220;
    int statsHeight = 130;  // Increased height for better layout

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::TOP_LEFT, "PlayerStats");

    DrawRectangle(statsX, statsY, statsWidth, statsHeight, Fade(DARKBLUE, 0.85f));
    DrawRectangleLines(statsX, statsY, statsWidth, statsHeight, SKYBLUE);

    // **UI ID: 1** - Player Stats Panel
    DrawText("PLAYER STATUS [ID:1]", statsX + 8, statsY + 8, 14, SKYBLUE);

    // Health bar - better positioned
    float healthPercent = (float)state.playerHealth / state.maxPlayerHealth;
    Color healthColor = healthPercent > 0.6f ? GREEN : (healthPercent > 0.3f ? YELLOW : RED);
    DrawText(TextFormat("HP: %d/%d", state.playerHealth, state.maxPlayerHealth),
             statsX + 8, statsY + 28, 12, healthColor);
    DrawRectangle(statsX + 8, statsY + 42, 200, 8, DARKGRAY);
    DrawRectangle(statsX + 8, statsY + 42, (int)(200 * healthPercent), 8, healthColor);

    // Mana bar - better positioned
    float manaPercent = (float)state.playerMana / state.maxPlayerMana;
    Color manaColor = manaPercent > 0.6f ? BLUE : (manaPercent > 0.3f ? SKYBLUE : PURPLE);
    DrawText(TextFormat("MP: %d/%d", state.playerMana, state.maxPlayerMana),
             statsX + 8, statsY + 55, 12, manaColor);
    DrawRectangle(statsX + 8, statsY + 69, 200, 8, DARKGRAY);
    DrawRectangle(statsX + 8, statsY + 69, (int)(200 * manaPercent), 8, manaColor);

    // Level and experience - better positioned
    DrawText(TextFormat("Level %d | XP: %d", state.playerLevel, state.playerExperience),
             statsX + 8, statsY + 85, 12, GOLD);

    // Equipment bonuses (if any) - better positioned
    if (state.inventorySystem) {
        auto totalStats = state.inventorySystem->getTotalBonuses();
        if (totalStats.damage > 0 || totalStats.armor > 0) {
            DrawText(TextFormat("Equipment: +%d DMG, +%d ARM", totalStats.damage, totalStats.armor),
                     statsX + 8, statsY + 105, 11, GREEN);
        }
    }
}

void UISystemManager::renderPerformanceDisplay(const GameState& state) {
    // **TOP-RIGHT ZONE** - Performance overlay - moved down to avoid overlap with other panels
    Rectangle zone = getZoneBounds(UIZone::TOP_RIGHT);

    // Draw FPS counter with ID tag - moved down
    int fpsX = (int)zone.x + zone.width - 70;
    int fpsY = (int)zone.y + 25;  // Moved down from +5 to +25

    DrawFPS(fpsX, fpsY);

    // Add ID tag next to FPS
    DrawText("[ID:12]", fpsX - 45, fpsY + 2, 8, WHITE);
}

void UISystemManager::renderControlsPanel(const GameState& state) {
    // **BOTTOM-LEFT ZONE** - Properly sized controls help
    Rectangle zoneBounds = getZoneBounds(UIZone::BOTTOM_LEFT);
    Rectangle zone = calculateNonOverlappingPosition(UIZone::BOTTOM_LEFT, zoneBounds.width - 10, 55);
    int panelX = (int)zone.x + 5;
    int panelY = (int)zone.y + 5;

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::BOTTOM_LEFT, "ControlsPanel");

    DrawRectangle(panelX, panelY, zone.width - 10, 55, Fade(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, zone.width - 10, 55, BLUE);
    DrawText("CONTROLS [ID:4]", panelX + 8, panelY + 8, 14, SKYBLUE);
    DrawText("WASD: Move | Mouse: Look | Space: Jump | LMB: Attack", panelX + 8, panelY + 25, 11, LIGHTGRAY);
    DrawText("E: Interact | I: Inventory | ESC: Menu | TAB: Testing", panelX + 8, panelY + 38, 11, LIGHTGRAY);
}

void UISystemManager::renderGameStats(const GameState& state) {
    // **BOTTOM-LEFT ZONE** - Properly positioned below controls with adequate spacing
    Rectangle zoneBounds = getZoneBounds(UIZone::BOTTOM_LEFT);
    Rectangle zone = calculateNonOverlappingPosition(UIZone::BOTTOM_LEFT, zoneBounds.width - 10, 95);
    int statsX = (int)zone.x + 5;
    int statsY = (int)zone.y + 70;  // Adequate spacing below controls panel

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::BOTTOM_LEFT, "GameStats");

    DrawRectangle(statsX, statsY, zone.width - 10, 95, Fade(BLACK, 0.8f));
    DrawRectangleLines(statsX, statsY, zone.width - 10, 95, GREEN);

    DrawText("GAME STATS [ID:5]", statsX + 8, statsY + 8, 14, LIME);
    DrawText(TextFormat("Score: %d", state.score), statsX + 8, statsY + 25, 12, WHITE);
    DrawText(TextFormat("Swings: %d", state.swingsPerformed), statsX + 8, statsY + 42, 12, WHITE);
    DrawText(TextFormat("Hits: %d", state.meleeHits), statsX + 8, statsY + 59, 12, WHITE);
    float accuracy = state.swingsPerformed > 0 ? (float)state.meleeHits / state.swingsPerformed * 100.0f : 0.0f;
    DrawText(TextFormat("Accuracy: %.1f%%", accuracy), statsX + 8, statsY + 76, 12, WHITE);
}

void UISystemManager::renderInteractionPrompt(const GameState& state, float currentTime) {
    if (!state.showInteractPrompt || state.isInDialog) return;
    
    // **INTERACTION ZONE** - Mid-screen left for prompts
    Rectangle zone = getZoneBounds(UIZone::INTERACTION);
    float alpha = 0.8f + sinf(currentTime * 3.0f) * 0.1f;
    
    DrawRectangle((int)zone.x, (int)zone.y, 350, 40, Fade(BLACK, alpha));
    DrawRectangleLines((int)zone.x, (int)zone.y, 350, 40, YELLOW);

    DrawText(state.interactPromptText.c_str(), (int)zone.x + 10, (int)zone.y + 5, 14, YELLOW);
    DrawText("[ID:11]", (int)zone.x + 300, (int)zone.y + 5, 10, WHITE);
    DrawText("[E] to interact", (int)zone.x + 10, (int)zone.y + 20, 12, WHITE);
    
    DrawCircle((int)zone.x + 320, (int)zone.y + 20, 3, YELLOW);
}

void UISystemManager::renderDiagnosticPanel(const GameState& state) {
    // **TOP-LEFT ZONE** - Properly sized diagnostic panels with clear separation
    Rectangle zone = getZoneBounds(UIZone::TOP_LEFT);
    int diagX = (int)zone.x + 5;
    int diagY = (int)zone.y + 5;

    // **TITLE PANEL** - Browserwind title
    DrawRectangle(diagX, diagY, zone.width - 10, 25, Fade(DARKBLUE, 0.9f));
    DrawRectangleLines(diagX, diagY, zone.width - 10, 25, SKYBLUE);
    DrawText("BROWSERWIND [ID:2]", diagX + 8, diagY + 6, 16, WHITE);

    // **GAME STATE PANEL** - Below title with proper spacing
    DrawRectangle(diagX, diagY + 35, zone.width - 10, 45, Fade(DARKGREEN, 0.85f));
    DrawRectangleLines(diagX, diagY + 35, zone.width - 10, 45, GREEN);

    // **UI ID: 10** - Game State Panel
    DrawText("STATUS [ID:10]", diagX + 8, diagY + 38, 12, WHITE);

    Color mouseStateColor = state.mouseReleased ? YELLOW : GREEN;
    const char* mouseStateText = state.mouseReleased ? "Mouse: FREE" : "Mouse: CAPTURED";
    DrawText(mouseStateText, diagX + 8, diagY + 42, 12, mouseStateColor);

    if (state.showEscMenu) {
        DrawText("Game: PAUSED", diagX + 8, diagY + 58, 12, RED);
        DrawText("ESC Menu Active", diagX + 8, diagY + 70, 10, RED);
    } else if (state.showInventoryWindow) {
        DrawText("Inventory: OPEN", diagX + 8, diagY + 58, 12, PURPLE);
        DrawText("Click items to interact", diagX + 8, diagY + 70, 10, PURPLE);
    } else {
        DrawText("Game: ACTIVE", diagX + 8, diagY + 58, 12, GREEN);
        DrawText("Press TAB for testing panel", diagX + 8, diagY + 70, 10, LIGHTGRAY);
    }
}

void UISystemManager::renderTestingStatus(const GameState& state) {
    // **TOP-RIGHT ZONE** - Properly sized testing summary within zone bounds
    Rectangle zoneBounds = getZoneBounds(UIZone::TOP_RIGHT);
    Rectangle zone = calculateNonOverlappingPosition(UIZone::TOP_RIGHT, zoneBounds.width - 10, 75);
    int testX = (int)zone.x + 5;
    int testY = (int)zone.y + 110;  // Moved down by another 15 pixels (total 50 pixels from original)

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::TOP_RIGHT, "TestingStatus");

    DrawRectangle(testX, testY, zone.width - 10, 75, Fade(BLACK, 0.85f));
    DrawRectangleLines(testX, testY, zone.width - 10, 75, BLUE);

    DrawText("TESTING STATUS [ID:3]", testX + 8, testY + 8, 13, SKYBLUE);

    // Calculate progress
    int workingFeatures = (state.testWASDMovement ? 1 : 0) + (state.testSpaceJump ? 1 : 0) +
                          (state.testMouseLook ? 1 : 0) + (state.testMeleeSwing ? 1 : 0) +
                          (state.testBuildingEntry ? 1 : 0) + (state.testNPCInteraction ? 1 : 0);

    Color statusColor = workingFeatures >= 5 ? GREEN : (workingFeatures >= 3 ? YELLOW : RED);
    DrawText(TextFormat("Progress: %d/6 Features", workingFeatures), testX + 8, testY + 25, 12, statusColor);

    // Show hint to open full panel
    DrawText("Press TAB for detailed checklist", testX + 8, testY + 45, 11, LIGHTGRAY);
    DrawText("Test: WASD, Mouse, Space, LMB, Doors, NPCs", testX + 8, testY + 58, 10, GRAY);
}

// ===== MODAL RENDERERS =====

void UISystemManager::renderDialogModal(const GameState& state) {
    // **FULL-SCREEN MODAL** - Front and center like ESC menu
    
    // **FULL-SCREEN DARK OVERLAY** - Same as ESC menu
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.6f));
    
    // **CENTERED DIALOG WINDOW** - Larger and more prominent
    int dialogWidth = 600;
    int dialogHeight = 250;
    int dialogX = (screenWidth_ - dialogWidth) / 2;
    int dialogY = (screenHeight_ - dialogHeight) / 2;
    
    // Dialog background with enhanced styling
    DrawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, Fade(DARKBLUE, 0.95f));
    DrawRectangleLines(dialogX, dialogY, dialogWidth, dialogHeight, SKYBLUE);
    DrawRectangleLines(dialogX + 2, dialogY + 2, dialogWidth - 4, dialogHeight - 4, WHITE);
    
    // **DIALOG TITLE**
    DrawText("CONVERSATION [ID:6]", dialogX + 20, dialogY + 15, 18, SKYBLUE);
    
    // **NPC DIALOG TEXT** - Wrapped and readable
    DrawText(state.dialogText.c_str(), dialogX + 20, dialogY + 45, 14, WHITE);
    
    // **DIALOG OPTIONS** - Enhanced button styling
    const int buttonY = dialogY + 120;
    const int buttonHeight = 35;
    const int buttonWidth = 170;
    const int buttonSpacing = 180;
    const int startX = dialogX + 20;
    
    Vector2 mousePos = GetMousePosition();
    
    for (int i = 0; i < state.numDialogOptions; i++) {
        int buttonX = startX + (i * buttonSpacing);
        
        bool isHovered = (mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight &&
                          mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth);
        
        Color buttonColor = Fade(BLUE, 0.7f);
        if (isHovered) {
            buttonColor = Fade(SKYBLUE, 0.9f);
            DrawRectangle(buttonX - 3, buttonY - 3, buttonWidth + 6, buttonHeight + 6, Fade(WHITE, 0.4f));
        }
        
        DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
        DrawRectangleLines(buttonX, buttonY, buttonWidth, buttonHeight, WHITE);
        
        DrawText(state.dialogOptions[i].c_str(), buttonX + 10, buttonY + 10, 12, WHITE);
        
        if (isHovered) {
            DrawText(">", buttonX - 15, buttonY + 10, 12, YELLOW);
        }
    }
    
    // **DIALOG CONTROLS HELP**
    DrawText("Click on an option to continue the conversation", dialogX + 20, dialogY + dialogHeight - 30, 12, LIGHTGRAY);
}

void UISystemManager::renderInventoryModal(const GameState& state) {
    // **FULL-SCREEN MODAL** - Dark overlay + centered window
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.5f));
    
    // Use existing inventory UI - it's already well-positioned
    renderInventoryUI(state);
}

void UISystemManager::renderEscMenuOverlay(GameState& state) {
    // **FULL-SCREEN OVERLAY** - Use existing ESC menu implementation
    renderEscMenu(state);
}

// ===== BACKGROUND LAYER HELPERS =====



Rectangle UISystemManager::positionInZone(UIZone zone, int width, int height, int offsetX, int offsetY) {
    Rectangle zoneBounds;
    switch (zone) {
        case UIZone::TOP_LEFT:
            zoneBounds = {static_cast<float>(10 + offsetX), static_cast<float>(10 + offsetY), (float)width, (float)height};
            break;
        case UIZone::TOP_RIGHT:
            zoneBounds = {static_cast<float>(800 - width - 10 + offsetX), static_cast<float>(10 + offsetY), (float)width, (float)height};  // Assuming 800px width
            break;
        case UIZone::CENTER:
            zoneBounds = {static_cast<float>(400 - width/2 + offsetX), static_cast<float>(300 - height/2 + offsetY), (float)width, (float)height};
            break;
        case UIZone::BOTTOM_LEFT:
            zoneBounds = {static_cast<float>(10 + offsetX), static_cast<float>(600 - height - 10 + offsetY), (float)width, (float)height};  // Assuming 600px height
            break;
        case UIZone::BOTTOM_RIGHT:
            zoneBounds = {static_cast<float>(800 - width - 10 + offsetX), static_cast<float>(600 - height - 10 + offsetY), (float)width, (float)height};
            break;
        case UIZone::INTERACTION:
            zoneBounds = {static_cast<float>(10 + offsetX), static_cast<float>(250 + offsetY), (float)width, (float)height};  // Mid-screen
            break;
        default:
            zoneBounds = {0, 0, (float)width, (float)height};
    }
    return zoneBounds;
}

bool UISystemManager::checkUICollision(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

void UISystemManager::reserveZone(UIZone zone, const std::string& elementName) {
    zoneReservations_[zone] = elementName;
}

void UISystemManager::releaseZone(UIZone zone) {
    zoneReservations_.erase(zone);
}

bool UISystemManager::isZoneOccupied(UIZone zone) const {
    return zoneReservations_.find(zone) != zoneReservations_.end();
}

// ===== DETAILED TESTING PANEL (TAB TOGGLE) =====

void UISystemManager::renderDetailedTestingPanel(const GameState& state, const std::string& locationText, Color locationColor) {
    // **CENTER-SCREEN MODAL** - Like inventory/ESC menu
    int panelWidth = 520;
    int panelHeight = 400;
    int panelX = (screenWidth_ - panelWidth) / 2;  // Centered horizontally
    int panelY = (screenHeight_ - panelHeight) / 2 - 30;  // Centered vertically
    
    // **FULL SCREEN OVERLAY** - Modal behavior
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.4f));
    
    // **MAIN PANEL** with enhanced styling
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(BLACK, 0.95f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, BLUE);
    DrawRectangleLines(panelX + 2, panelY + 2, panelWidth - 4, panelHeight - 4, SKYBLUE);
    
    int yOffset = panelY + 15;
    
    DrawText("BROWSERWIND - FEATURE TESTING CHECKLIST [ID:9]", panelX + 15, yOffset, 16, SKYBLUE);
    yOffset += 25;
    
    // **MOVEMENT & CONTROLS SECTION**
    DrawText("=== MOVEMENT & CONTROLS ===", panelX + 15, yOffset, 13, YELLOW);
    yOffset += 20;
    
    Color mouseColor = state.testMouseCaptured ? GREEN : RED;
    DrawText(TextFormat("Mouse Capture: %s", state.testMouseCaptured ? "WORKING" : "BROKEN"), panelX + 20, yOffset, 11, mouseColor);
    yOffset += 16;
    
    Color wasdColor = state.testWASDMovement ? GREEN : ORANGE;
    DrawText(TextFormat("WASD Movement: %s", state.testWASDMovement ? "TESTED" : "TEST W/A/S/D KEYS"), panelX + 20, yOffset, 11, wasdColor);
    yOffset += 16;
    
    Color jumpColor = state.testSpaceJump ? GREEN : ORANGE;
    DrawText(TextFormat("Space Jump: %s", state.testSpaceJump ? "TESTED" : "TEST SPACE KEY"), panelX + 20, yOffset, 11, jumpColor);
    yOffset += 16;
    
    Color lookColor = state.testMouseLook ? GREEN : ORANGE;
    DrawText(TextFormat("Mouse Look: %s", state.testMouseLook ? "TESTED" : "TEST MOUSE MOVEMENT"), panelX + 20, yOffset, 11, lookColor);
    yOffset += 22;
    
    // **COMBAT SYSTEM SECTION**
    DrawText("=== COMBAT SYSTEM ===", panelX + 15, yOffset, 13, YELLOW);
    yOffset += 20;
    
    Color swingColor = state.testMeleeSwing ? GREEN : ORANGE;
    DrawText(TextFormat("Melee Attack: %s", state.testMeleeSwing ? "TESTED" : "TEST LEFT MOUSE CLICK"), panelX + 20, yOffset, 11, swingColor);
    yOffset += 16;
    
    Color hitColor = state.testMeleeHitDetection ? GREEN : ORANGE;
    DrawText(TextFormat("Hit Detection: %s", state.testMeleeHitDetection ? "TESTED" : "HIT TARGETS WITH ATTACKS"), panelX + 20, yOffset, 11, hitColor);
    yOffset += 22;
    
    // **WORLD INTERACTION SECTION**
    DrawText("=== WORLD INTERACTION ===", panelX + 15, yOffset, 13, YELLOW);
    yOffset += 20;
    
    Color buildingColor = state.testBuildingEntry ? GREEN : ORANGE;
    DrawText(TextFormat("Building Entry: %s", state.testBuildingEntry ? "TESTED" : "OPEN A DOOR - WALK TO BUILDING"), panelX + 20, yOffset, 11, buildingColor);
    yOffset += 16;
    
    Color npcColor = state.testNPCInteraction ? GREEN : ORANGE;
    DrawText(TextFormat("NPC Dialog: %s", state.testNPCInteraction ? "TESTED" : "TALK TO AN NPC - PRESS E"), panelX + 20, yOffset, 11, npcColor);
    yOffset += 22;
    
    // **SYSTEM STATUS SECTION**
    DrawText("=== SYSTEM STATUS ===", panelX + 15, yOffset, 13, YELLOW);
    yOffset += 20;
    
    Color mouseStateColor = state.mouseReleased ? ORANGE : GREEN;
    DrawText(TextFormat("Mouse State: %s", state.mouseReleased ? "FREE" : "CAPTURED"), panelX + 20, yOffset, 11, mouseStateColor);
    yOffset += 16;
    
    DrawText(TextFormat("Location: %s", locationText.c_str()), panelX + 20, yOffset, 11, locationColor);
    yOffset += 16;
    
    Color dialogColor = state.isInDialog ? PURPLE : GRAY;
    DrawText(TextFormat("Dialog: %s", state.isInDialog ? "ACTIVE" : "INACTIVE"), panelX + 20, yOffset, 11, dialogColor);
    yOffset += 20;
    
    // **TESTING INSTRUCTIONS**
    DrawText("HOW TO TEST:", panelX + 15, yOffset, 11, WHITE);
    yOffset += 16;
    DrawText("  • Move around with WASD keys", panelX + 20, yOffset, 10, LIGHTGRAY);
    yOffset += 14;
    DrawText("  • Look around with mouse", panelX + 20, yOffset, 10, LIGHTGRAY);
    yOffset += 14;
    DrawText("  • Jump with SPACE", panelX + 20, yOffset, 10, LIGHTGRAY);
    yOffset += 14;
    DrawText("  • Attack with LEFT MOUSE BUTTON", panelX + 20, yOffset, 10, LIGHTGRAY);
    yOffset += 14;
    DrawText("  • Walk to buildings and OPEN DOORS", panelX + 20, yOffset, 10, LIGHTGRAY);
    yOffset += 14;
    DrawText("  • Find NPCs inside and TALK TO THEM (press E)", panelX + 20, yOffset, 10, LIGHTGRAY);
    
    // **CLOSE INSTRUCTIONS**
    DrawText("Press TAB again to close this testing panel", panelX + 15, panelY + panelHeight - 20, 12, YELLOW);
}

// ===== COLLISION DETECTION & PREVENTION =====

void UISystemManager::clearOverlaps() {
    // Clear zone reservations to allow re-positioning
    zoneReservations_.clear();
}

Rectangle UISystemManager::calculateNonOverlappingPosition(UIZone preferredZone, int width, int height) const {
    Rectangle zoneBounds = getZoneBounds(preferredZone);

    // Check if the zone is already occupied
    if (isZoneOccupied(preferredZone)) {
        // Try alternative zones in order of preference
        std::vector<UIZone> alternatives;

        if (preferredZone == UIZone::TOP_LEFT) {
            alternatives = {UIZone::TOP_RIGHT, UIZone::BOTTOM_LEFT};
        } else if (preferredZone == UIZone::TOP_RIGHT) {
            alternatives = {UIZone::TOP_LEFT, UIZone::BOTTOM_RIGHT};
        } else if (preferredZone == UIZone::BOTTOM_LEFT) {
            alternatives = {UIZone::BOTTOM_RIGHT, UIZone::TOP_LEFT};
        } else if (preferredZone == UIZone::BOTTOM_RIGHT) {
            alternatives = {UIZone::BOTTOM_LEFT, UIZone::TOP_RIGHT};
        } else {
            alternatives = {UIZone::TOP_LEFT, UIZone::TOP_RIGHT, UIZone::BOTTOM_LEFT, UIZone::BOTTOM_RIGHT};
        }

        for (UIZone altZone : alternatives) {
            if (!isZoneOccupied(altZone)) {
                Rectangle altBounds = getZoneBounds(altZone);
                if (width <= altBounds.width && height <= altBounds.height) {
                    return altBounds;
                }
            }
        }

        // If all zones are occupied, return the original zone (will overlap)
        return zoneBounds;
    }

    return zoneBounds;
}
