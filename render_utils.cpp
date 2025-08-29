// render_utils.cpp
#include "render_utils.h"
#include <iostream>
#include <cmath>

void renderBuildingInterior(const Building& building) {
    float interiorScale = 0.95f;
    Vector3 buildingSize = building.getSize();
    DrawCube(building.position,
             buildingSize.x * interiorScale,
             buildingSize.y * interiorScale,
             buildingSize.z * interiorScale,
             Fade(building.getColor(), 1.0f));

    Vector3 floorPos = {building.position.x, 0.0f, building.position.z};
    DrawCube(floorPos, buildingSize.x * 0.9f, 0.1f, buildingSize.z * 0.9f, DARKGRAY);

    Vector3 ceilingPos = {building.position.x, building.position.y + buildingSize.y/2 - 0.05f, building.position.z};
    DrawCube(ceilingPos, buildingSize.x * 0.9f, 0.1f, buildingSize.z * 0.9f, LIGHTGRAY);
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

    Vector3 toNPC = {
        npc.position.x - camera.position.x,
        npc.position.y - camera.position.y,
        npc.position.z - camera.position.z
    };
    float distance = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y + toNPC.z * toNPC.z);

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

void renderUI(Camera3D camera, float currentTime, const GameState& state, bool testBuildingCollision) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    if (!state.isInDialog && !state.mouseReleased) {
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
        DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
    } else if (state.isInDialog) {
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        DrawCircle(centerX, centerY, 8, Fade(WHITE, 0.5f));
        DrawCircleLines(centerX, centerY, 8, WHITE);
        DrawText("DIALOG", centerX - 25, centerY - 25, 10, YELLOW);
    } else if (state.mouseReleased) {
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        DrawRectangle(centerX - 60, centerY - 20, 120, 40, Fade(BLACK, 0.8f));
        DrawRectangleLines(centerX - 60, centerY - 20, 120, 40, YELLOW);
        DrawText("MOUSE FREE", centerX - 45, centerY - 15, 12, WHITE);
        DrawText("ESC: RECAPTURE", centerX - 55, centerY + 2, 10, YELLOW);
    }

    DrawFPS(10, 10);

    if (state.showInteractPrompt && !state.isInDialog) {
        float alpha = 0.8f + sinf(currentTime * 3.0f) * 0.1f;
        DrawRectangle(5, 280, 400, 50, Fade(BLACK, alpha));
        DrawRectangleLines(5, 280, 400, 50, YELLOW);

        DrawText(state.interactPromptText.c_str(), 15, 290, 14, YELLOW);
        DrawText("[E] to interact", 15, 305, 12, WHITE);

        DrawCircle(385, 305, 3, YELLOW);
    }

    DrawRectangle(5, 340, 300, 60, Fade(BLACK, 0.7f));
    DrawRectangleLines(5, 340, 300, 60, BLUE);
    DrawText("=== CONTROLS ===", 10, 345, 12, SKYBLUE);
    DrawText("WASD: Move | Mouse: Look | Space: Jump", 10, 360, 10, LIGHTGRAY);
    DrawText("LMB: Attack | E: Interact", 10, 375, 10, LIGHTGRAY);

    if (testBuildingCollision) {
        DrawRectangle(5, 405, 200, 25, Fade(RED, 0.8f));
        DrawRectangleLines(5, 405, 200, 25, WHITE);
        DrawText("⚠️  WALL COLLISION", 10, 410, 10, WHITE);
    }

    Color mouseStateColor = state.mouseReleased ? ORANGE : GREEN;
    const char* mouseStateText = state.mouseReleased ? "MOUSE FREE" : "MOUSE CAPTURED";
    DrawRectangle(5, 430, 150, 20, Fade(mouseStateColor, 0.7f));
    DrawRectangleLines(5, 430, 150, 20, WHITE);
    DrawText(mouseStateText, 10, 432, 10, WHITE);
}

void renderTestingPanel(const GameState& state, const std::string& locationText, Color locationColor) {
    int yOffset = 30;

    DrawRectangle(5, 25, 500, 400, Fade(BLACK, 0.8f));
    DrawRectangleLines(5, 25, 500, 400, BLUE);

    DrawText("🎮 BROWSERWIND - FEATURE STATUS", 15, yOffset, 18, SKYBLUE);
    yOffset += 25;

    DrawText("─── MOVEMENT & CONTROLS ───", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color mouseColor = state.testMouseCaptured ? GREEN : RED;
    DrawText(TextFormat("🖱️  Mouse Capture: %s", state.testMouseCaptured ? "✓ WORKING" : "✗ BROKEN"), 20, yOffset, 12, mouseColor);
    yOffset += 15;

    Color wasdColor = state.testWASDMovement ? GREEN : ORANGE;
    DrawText(TextFormat("🏃 WASD Movement: %s", state.testWASDMovement ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, wasdColor);
    yOffset += 15;

    Color jumpColor = state.testSpaceJump ? GREEN : ORANGE;
    DrawText(TextFormat("🦘 Space Jump: %s", state.testSpaceJump ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, jumpColor);
    yOffset += 15;

    Color lookColor = state.testMouseLook ? GREEN : ORANGE;
    DrawText(TextFormat("👁️  Mouse Look: %s", state.testMouseLook ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, lookColor);
    yOffset += 18;

    DrawText("─── COMBAT SYSTEM ───", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color swingColor = state.testMeleeSwing ? GREEN : ORANGE;
    DrawText(TextFormat("⚔️  Melee Attack: %s", state.testMeleeSwing ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, swingColor);
    yOffset += 15;

    Color hitColor = state.testMeleeHitDetection ? GREEN : ORANGE;
    DrawText(TextFormat("🎯 Hit Detection: %s", state.testMeleeHitDetection ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, hitColor);
    yOffset += 18;

    DrawText("─── WORLD INTERACTION ───", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color buildingColor = state.testBuildingEntry ? GREEN : ORANGE;
    DrawText(TextFormat("🏛️  Building Entry: %s", state.testBuildingEntry ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, buildingColor);
    yOffset += 15;

    Color npcColor = state.testNPCInteraction ? GREEN : ORANGE;
    DrawText(TextFormat("👥 NPC Dialog: %s", state.testNPCInteraction ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, npcColor);
    yOffset += 18;

    DrawText("─── SYSTEM STATUS ───", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color mouseStateColor = state.mouseReleased ? ORANGE : GREEN;
    DrawText(TextFormat("🖱️  Mouse State: %s", state.mouseReleased ? "FREE" : "CAPTURED"), 20, yOffset, 12, mouseStateColor);
    yOffset += 15;

    DrawText(TextFormat("📍 Location: %s", locationText.c_str()), 20, yOffset, 12, locationColor);
    yOffset += 15;

    Color dialogColor = state.isInDialog ? PURPLE : GRAY;
    DrawText(TextFormat("💬 Dialog: %s", state.isInDialog ? "ACTIVE" : "INACTIVE"), 20, yOffset, 12, dialogColor);
    yOffset += 20;

    DrawText("🎯 TEST ALL FEATURES:", 15, yOffset, 12, WHITE);
    yOffset += 15;
    DrawText("   WASD + Mouse + Space + LMB + E", 20, yOffset, 10, LIGHTGRAY);
    yOffset += 15;
    DrawText("   ESC: Toggle mouse capture", 20, yOffset, 10, LIGHTGRAY);
    yOffset += 20;
    DrawText("📊 STATUS: 🟢 WORKING | 🟠 UNTESTED | 🔴 BROKEN", 15, yOffset, 10, GRAY);
}

void renderGameStats(const GameState& state) {
    DrawRectangle(5, 450, 250, 100, Fade(BLACK, 0.8f));
    DrawRectangleLines(5, 450, 250, 100, GREEN);

    DrawText("📊 GAME STATISTICS", 15, 455, 16, LIME);
    DrawText(TextFormat("🏆 Score: %d points", state.score), 20, 475, 12, WHITE);
    DrawText(TextFormat("⚔️  Swings: %d", state.swingsPerformed), 20, 490, 12, WHITE);
    DrawText(TextFormat("🎯 Hits: %d", state.meleeHits), 20, 505, 12, WHITE);
    float accuracy = state.swingsPerformed > 0 ? (float)state.meleeHits / state.swingsPerformed * 100.0f : 0.0f;
    DrawText(TextFormat("📈 Accuracy: %.1f%%", accuracy), 20, 520, 12, WHITE);
    DrawText(TextFormat("🏢 Buildings: %d/2", state.testBuildingEntry ? 1 : 0), 20, 535, 12, BLUE);
}

void renderDialogWindow(const GameState& state) {
    DrawRectangle(30, 250, 540, 180, Fade(BLACK, 0.9f));
    DrawRectangleLines(30, 250, 540, 180, WHITE);

    DrawText("Conversation", 40, 260, 18, SKYBLUE);

    DrawText(state.dialogText.c_str(), 40, 285, 14, WHITE);

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

        Color buttonColor = Fade(BLUE, 0.7f);
        if (isHovered) {
            buttonColor = Fade(SKYBLUE, 0.9f);
            DrawRectangle(buttonX - 2, buttonY - 2, buttonWidth + 4, buttonHeight + 4, Fade(WHITE, 0.3f));
        }

        DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
        DrawRectangleLines(buttonX, buttonY, buttonWidth, buttonHeight, WHITE);

        DrawText(state.dialogOptions[i].c_str(), buttonX + 10, buttonY + 8, 12, WHITE);

        if (isHovered) {
            DrawText(">", buttonX - 15, buttonY + 8, 12, YELLOW);
        }
    }

    DrawText("Click on an option to continue", 40, 410, 12, LIGHTGRAY);

    DrawText(TextFormat("Mouse: %.0f, %.0f", mousePos.x, mousePos.y), 400, 410, 10, YELLOW);
}

void printFinalSummary(const GameState& state) {
    std::cout << "\n🏘️  TOWN EXPLORATION TEST SUMMARY:" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << (state.testMouseCaptured ? "✅" : "❌") << " Mouse Capture: " << (state.testMouseCaptured ? "WORKING" : "NOT WORKING") << std::endl;
    std::cout << (state.testWASDMovement ? "✅" : "❌") << " WASD Movement: " << (state.testWASDMovement ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testSpaceJump ? "✅" : "❌") << " Space Jump: " << (state.testSpaceJump ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testMouseLook ? "✅" : "❌") << " Mouse Look: " << (state.testMouseLook ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testMeleeSwing ? "✅" : "❌") << " Longsword Swing: " << (state.testMeleeSwing ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testMeleeHitDetection ? "✅" : "❌") << " Melee Hit Detection: " << (state.testMeleeHitDetection ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testBuildingEntry ? "✅" : "❌") << " Building Entry: " << (state.testBuildingEntry ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (state.testNPCInteraction ? "✅" : "❌") << " NPC Interaction: " << (state.testNPCInteraction ? "WORKING" : "NOT TESTED") << std::endl;

    int workingFeatures = (state.testMouseCaptured ? 1 : 0) + (state.testWASDMovement ? 1 : 0) +
                          (state.testSpaceJump ? 1 : 0) + (state.testMouseLook ? 1 : 0) +
                          (state.testMeleeSwing ? 1 : 0) + (state.testMeleeHitDetection ? 1 : 0) +
                          (state.testBuildingEntry ? 1 : 0) + (state.testNPCInteraction ? 1 : 0);

    std::cout << "\n🏆 Final Score: " << state.score << " points" << std::endl;
    std::cout << "🏘️  Features Working: " << workingFeatures << "/8" << std::endl;
    std::cout << "🗡️  Swings Performed: " << state.swingsPerformed << std::endl;
    std::cout << "🎯 Melee Hits: " << state.meleeHits << std::endl;
    std::cout << "🏢 Buildings Visited: " << (state.testBuildingEntry ? "YES" : "NO") << std::endl;
    std::cout << "👥 NPCs Talked To: " << (state.testNPCInteraction ? "YES" : "NO") << std::endl;
    std::cout << "💬 Dialog Conversations: " << (state.isInDialog ? "ACTIVE" : "COMPLETED") << std::endl;

    if (workingFeatures == 8) {
        std::cout << "🎉 ALL FEATURES WORKING PERFECTLY!" << std::endl;
    } else if (workingFeatures >= 6) {
        std::cout << "👍 MOST FEATURES WORKING WELL!" << std::endl;
    } else {
        std::cout << "⚠️  Some features need attention." << std::endl;
    }
}