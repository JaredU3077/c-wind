// render_utils.cpp
#include "render_utils.h"
#include "math_utils.h"
#include "ui_theme_optimized.h"
#include <iostream>
#include <cmath>

// Local utility function for color fading
Color fadeColor(Color color, float alpha) {
    return Color{
        static_cast<unsigned char>(color.r * alpha),
        static_cast<unsigned char>(color.g * alpha),
        static_cast<unsigned char>(color.b * alpha),
        static_cast<unsigned char>(color.a * alpha)
    };
}

void renderBuildingInterior(const Building& building) {
    // Simplified interior - just render floor and subtle ambient lighting
    Vector3 buildingSize = building.getSize();

    // Simple floor
    Vector3 floorPos = {building.position.x, building.position.y - buildingSize.y/2 + 0.02f, building.position.z};
    DrawCube(floorPos, buildingSize.x * 0.95f, 0.04f, buildingSize.z * 0.95f, DARKGRAY);

    // Subtle ambient lighting - very dim corner lights
    float cornerOffset = buildingSize.x * 0.35f;
    Vector3 corners[4] = {
        {building.position.x + cornerOffset, building.position.y, building.position.z + cornerOffset},
        {building.position.x - cornerOffset, building.position.y, building.position.z + cornerOffset},
        {building.position.x + cornerOffset, building.position.y, building.position.z - cornerOffset},
        {building.position.x - cornerOffset, building.position.y, building.position.z - cornerOffset}
    };

    for (int i = 0; i < 4; i++) {
        DrawSphere(corners[i], 0.2f, Fade(WHITE, 0.05f));
    }
}

void renderNPC(const NPC& npc, Camera3D camera, float currentTime) {
    DrawCylinder(npc.position, 0.8f, 0.5f, 1.6f, 12, npc.color);
    DrawCylinderWires(npc.position, 0.8f, 0.5f, 1.6f, 12, BLACK);

    Vector3 headPos = {npc.position.x, npc.position.y + 1.8f, npc.position.z};
    DrawSphere(headPos, 0.45f, npc.color);
    DrawSphereWires(headPos, 0.45f, 12, 8, BLACK);

    Vector3 leftArmPos = {npc.position.x - 0.6f, npc.position.y + 0.8f, npc.position.z};
    Vector3 rightArmPos = {npc.position.x + 0.6f, npc.position.y + 0.8f, npc.position.z};
    DrawCylinder(leftArmPos, 0.15f, 0.15f, 0.8f, 8, Fade(npc.color, 0.9f));
    DrawCylinder(rightArmPos, 0.15f, 0.15f, 0.8f, 8, Fade(npc.color, 0.9f));

    Vector3 leftLegPos = {npc.position.x - 0.25f, npc.position.y - 0.4f, npc.position.z};
    Vector3 rightLegPos = {npc.position.x + 0.25f, npc.position.y - 0.4f, npc.position.z};
    DrawCylinder(leftLegPos, 0.2f, 0.15f, 0.8f, 8, Fade(npc.color, 0.8f));
    DrawCylinder(rightLegPos, 0.2f, 0.15f, 0.8f, 8, Fade(npc.color, 0.8f));

    float distance = MathUtils::distance3D(npc.position, camera.position);

    if (distance <= npc.interactionRadius) {
        float pulse = 0.8f + sinf(currentTime * 6.0f) * 0.4f;
        Vector3 indicatorPos = {npc.position.x, npc.position.y + 3.0f, npc.position.z};
        DrawSphere(indicatorPos, 0.2f * pulse, GREEN);

        DrawCircle3D(npc.position, npc.interactionRadius, {0, 1, 0}, 90, Fade(GREEN, 0.4f));

        DrawCylinder(npc.position, 0.85f, 0.55f, 1.65f, 12, Fade(YELLOW, 0.2f));
    } else if (distance <= npc.interactionRadius * 1.8f) {
        DrawCircle3D(npc.position, npc.interactionRadius, {0, 1, 0}, 90, Fade(YELLOW, 0.15f));
    }
}

void renderProjectedLabels(Camera3D camera, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding) {
    auto objects = environment.getAllObjects();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    for (const auto& obj : objects) {
        if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
            Vector3 signPos = building->getDoorPosition();
            signPos.y += 2.2f;
            Vector2 screenPos = GetWorldToScreen(signPos, camera);
            if (screenPos.x > 0 && screenPos.x < screenWidth && screenPos.y > 0 && screenPos.y < screenHeight) {
                int textWidth = MeasureText(building->getName().c_str(), 20);
                DrawText(building->getName().c_str(), screenPos.x - textWidth / 2, screenPos.y - 20, 20, BLACK);
            }
        }
    }

    for (int n = 0; n < MAX_NPCS; n++) {
        bool shouldLabelNPC = false;
        if (isInBuilding) {
            if (currentBuilding == 0 && n == 0) shouldLabelNPC = true;
            if (currentBuilding == 1 && n == 1) shouldLabelNPC = true;
        }
        if (shouldLabelNPC) {
            Vector3 labelPos = npcs[n].position;
            labelPos.y += 2.5f;
            Vector2 screenPos = GetWorldToScreen(labelPos, camera);
            if (screenPos.x > 0 && screenPos.x < screenWidth && screenPos.y > 0 && screenPos.y < screenHeight) {
                int textWidth = MeasureText(npcs[n].name.c_str(), 18);
                DrawText(npcs[n].name.c_str(), screenPos.x - textWidth / 2, screenPos.y - 20, 18, WHITE);
            }
        }
    }
}

void renderUI([[maybe_unused]] Camera3D camera, float currentTime, const GameState& state, bool testBuildingCollision) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    if (!state.isInDialog && !state.mouseReleased) {
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        Color crosshairColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
        DrawLine(centerX - 10, centerY, centerX + 10, centerY, crosshairColor);
        DrawLine(centerX, centerY - 10, centerX, centerY + 10, crosshairColor);
    } else if (state.isInDialog) {
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
        DrawCircle(centerX, centerY, 8, fadeColor(accentColor, 0.5f));
        DrawCircleLines(centerX, centerY, 8, accentColor);
        DrawText("DIALOG", centerX - 25, centerY - 25, 10, accentColor);
    } else if (state.mouseReleased) {
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
        Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
        Color textColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);

        DrawRectangle(centerX - 60, centerY - 20, 120, 40, fadeColor(bgColor, 0.8f));
        DrawRectangleLines(centerX - 60, centerY - 20, 120, 40, accentColor);
        DrawText("MOUSE FREE", centerX - 45, centerY - 15, 12, textColor);
        DrawText("ESC: RECAPTURE", centerX - 55, centerY + 2, 10, accentColor);
    }

    DrawFPS(10, 10);

    if (state.showInteractPrompt && !state.isInDialog) {
        float alpha = 0.8f + sinf(currentTime * 3.0f) * 0.1f;
        Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
        Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
        Color textColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);

        DrawRectangle(5, 280, 400, 50, fadeColor(bgColor, alpha));
        DrawRectangleLines(5, 280, 400, 50, accentColor);

        DrawText(state.interactPromptText.c_str(), 15, 290, 14, accentColor);
        DrawText("[E] to interact", 15, 305, 12, textColor);

        DrawCircle(385, 305, 3, accentColor);
    }

    // Controls panel with theme colors
    {
        Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
        Color borderColor = UITypes::GetThemeColor(UITypes::ColorRole::BORDER);
        Color textPrimary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
        Color textSecondary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY);

        DrawRectangle(5, 340, 300, 60, fadeColor(bgColor, 0.7f));
        DrawRectangleLines(5, 340, 300, 60, borderColor);
        DrawText("=== CONTROLS ===", 10, 345, 12, textPrimary);
        DrawText("WASD: Move | Mouse: Look | Space: Jump", 10, 360, 10, textSecondary);
        DrawText("LMB: Attack | E: Interact", 10, 375, 10, textSecondary);
    }

    if (testBuildingCollision) {
        Color errorColor = UITypes::GetThemeColor(UITypes::ColorRole::ERROR);
        Color textColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);

        DrawRectangle(5, 405, 200, 25, fadeColor(errorColor, 0.8f));
        DrawRectangleLines(5, 405, 200, 25, textColor);
        DrawText("⚠️  WALL COLLISION", 10, 410, 10, textColor);
    }

    // Mouse state indicator with theme colors
    Color mouseStateColor = state.mouseReleased ?
        UITypes::GetThemeColor(UITypes::ColorRole::WARNING) :
        UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS);
    const char* mouseStateText = state.mouseReleased ? "MOUSE FREE" : "MOUSE CAPTURED";
    Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
    Color textColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);

    DrawRectangle(5, 430, 150, 20, fadeColor(mouseStateColor, 0.7f));
    DrawRectangleLines(5, 430, 150, 20, textColor);
    DrawText(mouseStateText, 10, 432, 10, textColor);
}

void renderTestingPanel(const GameState& state, const std::string& locationText, Color locationColor) {
    int yOffset = 30;

    // Use theme colors for consistent appearance
    Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
    Color borderColor = UITypes::GetThemeColor(UITypes::ColorRole::BORDER);
    Color textPrimary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
    Color textSecondary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY);
    Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
    Color successColor = UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS);
    Color warningColor = UITypes::GetThemeColor(UITypes::ColorRole::WARNING);
    Color errorColor = UITypes::GetThemeColor(UITypes::ColorRole::ERROR);

    DrawRectangle(5, 25, 500, 400, fadeColor(bgColor, 0.8f));
    DrawRectangleLines(5, 25, 500, 400, borderColor);

    DrawText("🎮 BROWSERWIND - FEATURE STATUS", 15, yOffset, 18, textPrimary);
    yOffset += 25;

    DrawText("=== MOVEMENT & CONTROLS ===", 15, yOffset, 14, accentColor);
    yOffset += 18;

    Color mouseColor = state.testMouseCaptured ? successColor : errorColor;
    DrawText(TextFormat("🖱️  Mouse Capture: %s", state.testMouseCaptured ? "✓ WORKING" : "✗ BROKEN"), 20, yOffset, 12, mouseColor);
    yOffset += 15;

    Color wasdColor = state.testWASDMovement ? successColor : warningColor;
    DrawText(TextFormat("🏃 WASD Movement: %s", state.testWASDMovement ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, wasdColor);
    yOffset += 15;

    Color jumpColor = state.testSpaceJump ? successColor : warningColor;
    DrawText(TextFormat("🦘 Space Jump: %s", state.testSpaceJump ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, jumpColor);
    yOffset += 15;

    Color lookColor = state.testMouseLook ? successColor : warningColor;
    DrawText(TextFormat("👁️  Mouse Look: %s", state.testMouseLook ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, lookColor);
    yOffset += 18;

    DrawText("=== COMBAT SYSTEM ===", 15, yOffset, 14, accentColor);
    yOffset += 18;

    Color swingColor = state.testMeleeSwing ? successColor : warningColor;
    DrawText(TextFormat("⚔️  Melee Attack: %s", state.testMeleeSwing ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, swingColor);
    yOffset += 15;

    Color hitColor = state.testMeleeHitDetection ? successColor : warningColor;
    DrawText(TextFormat("Hit Detection: %s", state.testMeleeHitDetection ? "TESTED" : "UNTESTED"), 20, yOffset, 12, hitColor);
    yOffset += 18;

    DrawText("=== WORLD INTERACTION ===", 15, yOffset, 14, accentColor);
    yOffset += 18;

    Color buildingColor = state.testBuildingEntry ? successColor : warningColor;
    DrawText(TextFormat("🏛️  Building Entry: %s", state.testBuildingEntry ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, buildingColor);
    yOffset += 15;

    Color npcColor = state.testNPCInteraction ? successColor : warningColor;
    DrawText(TextFormat("👥 NPC Dialog: %s", state.testNPCInteraction ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, npcColor);
    yOffset += 18;

    DrawText("=== SYSTEM STATUS ===", 15, yOffset, 14, accentColor);
    yOffset += 18;

    Color mouseStateColor = state.mouseReleased ? warningColor : successColor;
    DrawText(TextFormat("🖱️  Mouse State: %s", state.mouseReleased ? "FREE" : "CAPTURED"), 20, yOffset, 12, mouseStateColor);
    yOffset += 15;

    DrawText(TextFormat("📍 Location: %s", locationText.c_str()), 20, yOffset, 12, locationColor);
    yOffset += 15;

    Color dialogColor = state.isInDialog ? UITypes::GetThemeColor(UITypes::ColorRole::INFO) : textSecondary;
    DrawText(TextFormat("💬 Dialog: %s", state.isInDialog ? "ACTIVE" : "INACTIVE"), 20, yOffset, 12, dialogColor);
    yOffset += 20;

    DrawText("TEST ALL FEATURES:", 15, yOffset, 12, textPrimary);
    yOffset += 15;
    DrawText("   WASD + Mouse + Space + LMB + E", 20, yOffset, 10, textSecondary);
    yOffset += 15;
    DrawText("   ESC: Close UI/Toggle mouse | I: Inventory", 20, yOffset, 10, textSecondary);
    yOffset += 20;
    DrawText("📊 STATUS: 🟢 WORKING | 🟠 UNTESTED | 🔴 BROKEN", 15, yOffset, 10, textSecondary);
}

void renderGameStats(const GameState& state) {
    // Use theme colors for consistent appearance
    Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
    Color borderColor = UITypes::GetThemeColor(UITypes::ColorRole::BORDER);
    Color textPrimary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
    Color textSecondary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY);
    Color successColor = UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS);
    Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);

    DrawRectangle(5, 450, 250, 100, fadeColor(bgColor, 0.8f));
    DrawRectangleLines(5, 450, 250, 100, borderColor);

    DrawText("📊 GAME STATISTICS", 15, 455, 16, textPrimary);
    DrawText(TextFormat("🏆 Score: %d points", state.score), 20, 475, 12, textSecondary);
    DrawText(TextFormat("⚔️  Swings: %d", state.swingsPerformed), 20, 490, 12, textSecondary);
    DrawText(TextFormat("Hits: %d", state.meleeHits), 20, 505, 12, textSecondary);
    float accuracy = state.swingsPerformed > 0 ? (float)state.meleeHits / state.swingsPerformed * 100.0f : 0.0f;
    DrawText(TextFormat("📈 Accuracy: %.1f%%", accuracy), 20, 520, 12, textSecondary);
    DrawText(TextFormat("🏢 Buildings: %d/2", state.testBuildingEntry ? 1 : 0), 20, 535, 12, accentColor);
}

void renderDialogWindow(const GameState& state) {
    // Use theme colors for consistent appearance
    Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
    Color borderColor = UITypes::GetThemeColor(UITypes::ColorRole::BORDER);
    Color textPrimary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
    Color textSecondary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY);
    Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
    Color hoverColor = UITypes::GetThemeColor(UITypes::ColorRole::HOVER);

    DrawRectangle(30, 250, 540, 180, fadeColor(bgColor, 0.9f));
    DrawRectangleLines(30, 250, 540, 180, borderColor);

    DrawText("Conversation", 40, 260, 18, textPrimary);

    DrawText(state.dialogText.c_str(), 40, 285, 14, textSecondary);

    const int buttonY = 320;
    const int buttonHeight = 30;
    const int buttonWidth = 150;
    const int buttonSpacing = 170;
    const int startX = 50;

    Vector2 mousePos = GetMousePosition();

    for (int i = 0; i < state.numDialogOptions; i++) {
        int buttonX = startX + (i * buttonSpacing);

        bool isHovered = (mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight &&
                          mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth);

        Color buttonColor = fadeColor(accentColor, 0.7f);
        if (isHovered) {
            buttonColor = fadeColor(accentColor, 0.9f);
            DrawRectangle(buttonX - 2, buttonY - 2, buttonWidth + 4, buttonHeight + 4, fadeColor(hoverColor, 0.3f));
        }

        DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
        DrawRectangleLines(buttonX, buttonY, buttonWidth, buttonHeight, borderColor);

        DrawText(state.dialogOptions[i].c_str(), buttonX + 10, buttonY + 8, 12, textPrimary);

        if (isHovered) {
            DrawText(">", buttonX - 15, buttonY + 8, 12, accentColor);
        }
    }

    DrawText("Click on an option to continue", 40, 410, 12, textSecondary);

    DrawText(TextFormat("Mouse: %.0f, %.0f", mousePos.x, mousePos.y), 400, 410, 10, accentColor);
}

void renderInventoryUI(const GameState& state) {
    // Use the new component-based inventory UI system
    extern void renderInventoryComponent(const GameState& state);
    renderInventoryComponent(state);
}

void renderPlayerStatsUI(const GameState& state) {
    // **TOP-LEFT PLAYER STATS** - Always visible
    int statsX = 10;
    int statsY = 180;  // Below existing UI
    int statsWidth = 200;
    int statsHeight = 120;

    // Use theme colors for consistent appearance
    Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
    Color borderColor = UITypes::GetThemeColor(UITypes::ColorRole::BORDER);
    Color textPrimary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
    Color healthColor = UITypes::GetThemeColor(UITypes::ColorRole::HEALTH);
    Color manaColor = UITypes::GetThemeColor(UITypes::ColorRole::MANA);
    Color experienceColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE);

    DrawRectangle(statsX, statsY, statsWidth, statsHeight, fadeColor(bgColor, 0.8f));
    DrawRectangleLines(statsX, statsY, statsWidth, statsHeight, borderColor);

    DrawText("PLAYER STATUS", statsX + 10, statsY + 5, 14, textPrimary);

    // Health bar
    float healthPercent = (float)state.playerHealth / state.maxPlayerHealth;
    DrawText(TextFormat("HP: %d/%d", state.playerHealth, state.maxPlayerHealth),
             statsX + 10, statsY + 25, 12, healthColor);
    DrawRectangle(statsX + 10, statsY + 40, 180, 8, fadeColor(bgColor, 0.6f));
    DrawRectangle(statsX + 10, statsY + 40, (int)(180 * healthPercent), 8, healthColor);

    // Mana bar
    float manaPercent = (float)state.playerMana / state.maxPlayerMana;
    DrawText(TextFormat("MP: %d/%d", state.playerMana, state.maxPlayerMana),
             statsX + 10, statsY + 55, 12, manaColor);
    DrawRectangle(statsX + 10, statsY + 70, 180, 8, fadeColor(bgColor, 0.6f));
    DrawRectangle(statsX + 10, statsY + 70, (int)(180 * manaPercent), 8, manaColor);

    // Level and experience
    DrawText(TextFormat("Level %d", state.playerLevel), statsX + 10, statsY + 85, 12, experienceColor);
    DrawText(TextFormat("XP: %d", state.playerExperience), statsX + 100, statsY + 85, 12, experienceColor);

    // Equipment bonuses (if any)
    if (state.inventorySystem) {
        auto totalStats = state.inventorySystem->getTotalBonuses();
        if (totalStats.damage > 0 || totalStats.armor > 0) {
            Color successColor = UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS);
            DrawText(TextFormat("DMG+%d ARM+%d", totalStats.damage, totalStats.armor),
                     statsX + 10, statsY + 100, 10, successColor);
        }
    }
}

void printFinalSummary(const GameState& state) {
    std::cout << "\nTOWN EXPLORATION TEST SUMMARY:" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << (state.testMouseCaptured ? "[OK]" : "[X]") << " Mouse Capture: " << (state.testMouseCaptured ? "WORKING" : "NOT WORKING") << std::endl;
    std::cout << (state.testWASDMovement ? "[OK]" : "[X]") << " WASD Movement: " << (state.testWASDMovement ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testSpaceJump ? "[OK]" : "[X]") << " Space Jump: " << (state.testSpaceJump ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testMouseLook ? "[OK]" : "[X]") << " Mouse Look: " << (state.testMouseLook ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testMeleeSwing ? "[OK]" : "[X]") << " Longsword Swing: " << (state.testMeleeSwing ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testMeleeHitDetection ? "[OK]" : "[X]") << " Melee Hit Detection: " << (state.testMeleeHitDetection ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testBuildingEntry ? "[OK]" : "[X]") << " Building Entry: " << (state.testBuildingEntry ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testNPCInteraction ? "[OK]" : "[X]") << " NPC Interaction: " << (state.testNPCInteraction ? "WORKING" : "NOT TESTED") << std::endl;

    int workingFeatures = (state.testMouseCaptured ? 1 : 0) + (state.testWASDMovement ? 1 : 0) +
                          (state.testSpaceJump ? 1 : 0) + (state.testMouseLook ? 1 : 0) +
                          (state.testMeleeSwing ? 1 : 0) + (state.testMeleeHitDetection ? 1 : 0) +
                          (state.testBuildingEntry ? 1 : 0) + (state.testNPCInteraction ? 1 : 0);

    std::cout << "\n🏆 Final Score: " << state.score << " points" << std::endl;
    std::cout << "Features Working: " << workingFeatures << "/8" << std::endl;
    std::cout << "🗡️  Swings Performed: " << state.swingsPerformed << std::endl;
    std::cout << "Melee Hits: " << state.meleeHits << std::endl;
    std::cout << "🏢 Buildings Visited: " << (state.testBuildingEntry ? "YES" : "NO") << std::endl;
    std::cout << "👥 NPCs Talked To: " << (state.testNPCInteraction ? "YES" : "NO") << std::endl;
    std::cout << "💬 Dialog Conversations: " << (state.isInDialog ? "ACTIVE" : "COMPLETED") << std::endl;

    if (workingFeatures == 8) {
        std::cout << "ALL FEATURES WORKING PERFECTLY!" << std::endl;
    } else if (workingFeatures >= 6) {
        std::cout << "MOST FEATURES WORKING WELL!" << std::endl;
    } else {
        std::cout << "Some features need attention." << std::endl;
    }
}

void renderEscMenu(GameState& state) {  // **NON-CONST** to update selectedMenuOption
    if (!state.showEscMenu) return;

    // **FULL SCREEN OVERLAY** - Dark background
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Use theme colors for consistent appearance
    Color bgColor = UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND);
    Color surfaceColor = UITypes::GetThemeColor(UITypes::ColorRole::SURFACE);
    Color borderColor = UITypes::GetThemeColor(UITypes::ColorRole::BORDER);
    Color textPrimary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
    Color textSecondary = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY);
    Color accentColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
    Color warningColor = UITypes::GetThemeColor(UITypes::ColorRole::WARNING);

    DrawRectangle(0, 0, screenWidth, screenHeight, fadeColor(bgColor, 0.7f));

    // **CENTERED MENU WINDOW**
    int menuWidth = 400;
    int menuHeight = 300;
    int menuX = (screenWidth - menuWidth) / 2;
    int menuY = (screenHeight - menuHeight) / 2;

    // Menu background
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, fadeColor(surfaceColor, 0.9f));
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, borderColor);

    // **TITLE**
    DrawText("= BROWSERWIND PAUSED = [ID:8]", menuX + 80, menuY + 20, 20, textPrimary);
    
    // **MENU OPTIONS WITH MOUSE HOVER DETECTION**
    const char* menuOptions[] = {"Resume Game", "Save Game", "Load Game", "Quit Game"};
    int optionY = menuY + 80;
    int optionSpacing = 50;
    Vector2 mousePos = GetMousePosition();

    // **DETECT HOVERED OPTION** - Update selection based on mouse position
    int hoveredOption = -1;
    for (int i = 0; i < 4; i++) {
        int buttonY = optionY + i * optionSpacing - 5;
        int buttonHeight = 40;

        if (mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuWidth - 20 &&
            mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight) {
            hoveredOption = i;
            state.selectedMenuOption = i;  // **AUTO-UPDATE** selection on hover
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        bool isSelected = (i == state.selectedMenuOption);
        Color optionColor = isSelected ? accentColor : textPrimary;
        Color buttonBgColor = isSelected ? fadeColor(accentColor, 0.3f) : fadeColor(surfaceColor, 0.1f);

        // **ENHANCED HIGHLIGHT** for hovered/selected option
        if (isSelected) {
            DrawRectangle(menuX + 15, optionY + i * optionSpacing - 8, menuWidth - 30, 44, buttonBgColor);
            DrawRectangleLines(menuX + 15, optionY + i * optionSpacing - 8, menuWidth - 30, 44, accentColor);
        }

        // **OPTION TEXT WITH HOVER EFFECT**
        const char* prefix = isSelected ? "> " : "  ";
        int fontSize = isSelected ? 20 : 18;  // **BIGGER TEXT** when hovered
        DrawText(TextFormat("%s%s", prefix, menuOptions[i]),
                 menuX + 40, optionY + i * optionSpacing, fontSize, optionColor);
    }

    // **DYNAMIC CONTROLS HELP**
    if (hoveredOption >= 0) {
        const char* hoverHints[] = {
            "Click to resume your adventure!",
            "Click to save your current progress!",
            "Click to load your saved progress!",
            "Click to exit Browserwind!"
        };
        DrawText(hoverHints[hoveredOption], menuX + 30, menuY + menuHeight - 40, 12, accentColor);
    } else {
        DrawText("Hover over options to see details | Click to select | ESC to resume", menuX + 30, menuY + menuHeight - 40, 12, textSecondary);
    }
}