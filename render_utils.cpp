// render_utils.cpp
#include "render_utils.h"
#include <iostream>
#include <cmath>

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

    DrawText("=== MOVEMENT & CONTROLS ===", 15, yOffset, 14, YELLOW);
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

    DrawText("=== COMBAT SYSTEM ===", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color swingColor = state.testMeleeSwing ? GREEN : ORANGE;
    DrawText(TextFormat("⚔️  Melee Attack: %s", state.testMeleeSwing ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, swingColor);
    yOffset += 15;

    Color hitColor = state.testMeleeHitDetection ? GREEN : ORANGE;
    DrawText(TextFormat("Hit Detection: %s", state.testMeleeHitDetection ? "TESTED" : "UNTESTED"), 20, yOffset, 12, hitColor);
    yOffset += 18;

    DrawText("=== WORLD INTERACTION ===", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color buildingColor = state.testBuildingEntry ? GREEN : ORANGE;
    DrawText(TextFormat("🏛️  Building Entry: %s", state.testBuildingEntry ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, buildingColor);
    yOffset += 15;

    Color npcColor = state.testNPCInteraction ? GREEN : ORANGE;
    DrawText(TextFormat("👥 NPC Dialog: %s", state.testNPCInteraction ? "✓ TESTED" : "⏳ UNTESTED"), 20, yOffset, 12, npcColor);
    yOffset += 18;

    DrawText("=== SYSTEM STATUS ===", 15, yOffset, 14, YELLOW);
    yOffset += 18;

    Color mouseStateColor = state.mouseReleased ? ORANGE : GREEN;
    DrawText(TextFormat("🖱️  Mouse State: %s", state.mouseReleased ? "FREE" : "CAPTURED"), 20, yOffset, 12, mouseStateColor);
    yOffset += 15;

    DrawText(TextFormat("📍 Location: %s", locationText.c_str()), 20, yOffset, 12, locationColor);
    yOffset += 15;

    Color dialogColor = state.isInDialog ? PURPLE : GRAY;
    DrawText(TextFormat("💬 Dialog: %s", state.isInDialog ? "ACTIVE" : "INACTIVE"), 20, yOffset, 12, dialogColor);
    yOffset += 20;

    DrawText("TEST ALL FEATURES:", 15, yOffset, 12, WHITE);
    yOffset += 15;
    DrawText("   WASD + Mouse + Space + LMB + E", 20, yOffset, 10, LIGHTGRAY);
    yOffset += 15;
    DrawText("   ESC: Close UI/Toggle mouse | I: Inventory", 20, yOffset, 10, LIGHTGRAY);
    yOffset += 20;
    DrawText("📊 STATUS: 🟢 WORKING | 🟠 UNTESTED | 🔴 BROKEN", 15, yOffset, 10, GRAY);
}

void renderGameStats(const GameState& state) {
    DrawRectangle(5, 450, 250, 100, Fade(BLACK, 0.8f));
    DrawRectangleLines(5, 450, 250, 100, GREEN);

    DrawText("📊 GAME STATISTICS", 15, 455, 16, LIME);
    DrawText(TextFormat("🏆 Score: %d points", state.score), 20, 475, 12, WHITE);
    DrawText(TextFormat("⚔️  Swings: %d", state.swingsPerformed), 20, 490, 12, WHITE);
    DrawText(TextFormat("Hits: %d", state.meleeHits), 20, 505, 12, WHITE);
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

void renderInventoryUI(const GameState& state) {
    if (!state.inventorySystem) return;
    
    // **INTERACTIVE VISUAL INVENTORY** - Center of screen
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int invWidth = 500;
    int invHeight = 400;
    int invX = (screenWidth - invWidth) / 2;
    int invY = (screenHeight - invHeight) / 2;
    
    // Get mouse position for interaction
    Vector2 mousePos = GetMousePosition();
    bool mouseInInventory = (mousePos.x >= invX && mousePos.x <= invX + invWidth && 
                           mousePos.y >= invY && mousePos.y <= invY + invHeight);
    
    // Background window
    DrawRectangle(invX, invY, invWidth, invHeight, Fade(BLACK, 0.9f));
    DrawRectangleLines(invX, invY, invWidth, invHeight, GOLD);
    
    // Title
    DrawText("🎒 ADVENTURER'S INVENTORY", invX + 20, invY + 10, 18, GOLD);
    
    // Weight and slots info
    auto& inventory = state.inventorySystem->getInventory();
    DrawText(TextFormat("Weight: %.1f/%.0f kg", inventory.getCurrentWeight(), inventory.getMaxWeight()),
             invX + 20, invY + 35, 14, inventory.isOverweight() ? RED : WHITE);
    DrawText(TextFormat("Slots: %d/%d", inventory.getUsedSlots(), inventory.getMaxSlots()),
             invX + 250, invY + 35, 14, inventory.isFull() ? RED : WHITE);
    
    // **INTERACTIVE ITEM LIST**
    int yPos = invY + 60;
    int itemCount = 0;
    const int maxDisplayItems = 12;  // Limit display to prevent overflow
    const int itemHeight = 18;  // Height of each item row
    
    DrawText("=== ITEMS ===", invX + 20, yPos, 14, YELLOW);
    yPos += 25;
    
    std::shared_ptr<MysticalItem> hoveredItem = nullptr;
    
    for (const auto& item : inventory.getAllItems()) {
        if (itemCount >= maxDisplayItems) break;
        
        // **MOUSE HOVER DETECTION**
        Rectangle itemRect = {(float)(invX + 20), (float)yPos, 450.0f, (float)itemHeight};
        bool isHovered = CheckCollisionPointRec(mousePos, itemRect) && mouseInInventory;
        
        // **HOVER HIGHLIGHTING**
        if (isHovered) {
            DrawRectangle(invX + 20, yPos - 2, 450, itemHeight + 4, Fade(YELLOW, 0.3f));
            hoveredItem = item;  // Store for tooltip
        }
        
        // **CLICK DETECTION**
        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            std::cout << "Clicked on item: " << item->getName() << std::endl;
            // TODO: Add item interaction logic (equip/use/examine)
        }
        
        // Item rarity color
        Color itemColor = WHITE;
        switch (item->getRarity()) {
            case ItemRarity::UNCOMMON: itemColor = GREEN; break;
            case ItemRarity::RARE: itemColor = BLUE; break;
            case ItemRarity::EPIC: itemColor = PURPLE; break;
            case ItemRarity::LEGENDARY: itemColor = ORANGE; break;
            case ItemRarity::ARTIFACT: itemColor = RED; break;
            default: itemColor = WHITE; break;
        }
        
        std::string itemDisplay = item->getDisplayName();
        if (itemDisplay.length() > 35) {
            itemDisplay = itemDisplay.substr(0, 32) + "...";
        }
        
        DrawText(itemDisplay.c_str(), invX + 25, yPos, 12, itemColor);
        DrawText(TextFormat("%.1fkg %dg", item->getWeight() * item->getStackSize(), 
                           ItemUtils::calculateItemValue(*item)), 
                 invX + 350, yPos, 10, LIGHTGRAY);
        
        yPos += itemHeight;
        itemCount++;
    }
    
    if (inventory.getAllItems().size() > maxDisplayItems) {
        DrawText(TextFormat("... and %zu more items", inventory.getAllItems().size() - maxDisplayItems),
                 invX + 25, yPos, 10, GRAY);
    }
    
    // Equipment section
    yPos = invY + 220;
    DrawText("=== EQUIPPED GEAR ===", invX + 20, yPos, 14, YELLOW);
    yPos += 25;
    
    auto& equipment = state.inventorySystem->getEquipment();
    auto totalStats = equipment.getTotalStats();
    
    DrawText(TextFormat("⚔️ Total Damage: +%d", totalStats.damage), invX + 25, yPos, 12, totalStats.damage > 0 ? GREEN : GRAY);
    yPos += 18;
    DrawText(TextFormat("🛡️ Total Armor: +%d", totalStats.armor), invX + 25, yPos, 12, totalStats.armor > 0 ? GREEN : GRAY);
    yPos += 18;
    DrawText(TextFormat("Health Bonus: +%d", totalStats.health), invX + 25, yPos, 12, totalStats.health > 0 ? GREEN : GRAY);
    yPos += 18;
    DrawText(TextFormat("Mana Bonus: +%d", totalStats.mana), invX + 25, yPos, 12, totalStats.mana > 0 ? BLUE : GRAY);
    
    // Equipment slots display
    int equipY = invY + 60;
    DrawText("=== EQUIPMENT SLOTS ===", invX + 280, equipY, 12, YELLOW);
    equipY += 20;
    
    // Show key equipment slots
    auto mainHand = equipment.getEquippedItem(EquipmentSlot::MAIN_HAND);
    auto chest = equipment.getEquippedItem(EquipmentSlot::CHEST);
    auto feet = equipment.getEquippedItem(EquipmentSlot::FEET);
    
    DrawText(TextFormat("Main Hand: %s", mainHand ? mainHand->getName().c_str() : "[Empty]"),
             invX + 285, equipY, 10, mainHand ? GREEN : GRAY);
    equipY += 15;
    DrawText(TextFormat("Chest: %s", chest ? chest->getName().c_str() : "[Empty]"),
             invX + 285, equipY, 10, chest ? GREEN : GRAY);
    equipY += 15;
    DrawText(TextFormat("Feet: %s", feet ? feet->getName().c_str() : "[Empty]"),
             invX + 285, equipY, 10, feet ? GREEN : GRAY);
    
    // **ITEM TOOLTIP** - Show detailed info when hovering
    if (hoveredItem) {
        // Tooltip window
        int tooltipWidth = 300;
        int tooltipHeight = 150;
        int tooltipX = mousePos.x + 15;  // Offset from mouse
        int tooltipY = mousePos.y - tooltipHeight - 10;
        
        // Keep tooltip on screen
        if (tooltipX + tooltipWidth > screenWidth) tooltipX = mousePos.x - tooltipWidth - 15;
        if (tooltipY < 0) tooltipY = mousePos.y + 20;
        
        // Tooltip background
        DrawRectangle(tooltipX, tooltipY, tooltipWidth, tooltipHeight, Fade(BLACK, 0.95f));
        DrawRectangleLines(tooltipX, tooltipY, tooltipWidth, tooltipHeight, WHITE);
        
        // Item name with rarity color
        Color nameColor = WHITE;
        switch (hoveredItem->getRarity()) {
            case ItemRarity::UNCOMMON: nameColor = GREEN; break;
            case ItemRarity::RARE: nameColor = BLUE; break;
            case ItemRarity::EPIC: nameColor = PURPLE; break;
            case ItemRarity::LEGENDARY: nameColor = ORANGE; break;
            case ItemRarity::ARTIFACT: nameColor = RED; break;
            default: nameColor = WHITE; break;
        }
        
        DrawText(hoveredItem->getName().c_str(), tooltipX + 10, tooltipY + 10, 14, nameColor);
        DrawText(TextFormat("[%s]", ItemUtils::rarityToString(hoveredItem->getRarity()).c_str()), 
                 tooltipX + 10, tooltipY + 25, 10, nameColor);
        
        // Item stats
        int tooltipYPos = tooltipY + 45;
        DrawText(TextFormat("Weight: %.1f kg", hoveredItem->getWeight()), tooltipX + 10, tooltipYPos, 10, WHITE);
        tooltipYPos += 15;
        DrawText(TextFormat("Value: %d gold", ItemUtils::calculateItemValue(*hoveredItem)), 
                 tooltipX + 10, tooltipYPos, 10, GOLD);
        tooltipYPos += 15;
        
        // Type-specific info
        if (hoveredItem->getType() == ItemType::WEAPON) {
            auto weapon = std::dynamic_pointer_cast<EnchantedWeapon>(hoveredItem);
            if (weapon) {
                DrawText(TextFormat("Damage: %d", weapon->getStats().damage), tooltipX + 10, tooltipYPos, 10, RED);
                tooltipYPos += 15;
                DrawText(TextFormat("Durability: %.0f/%.0f", weapon->getDurability(), weapon->getMaxDurability()), 
                         tooltipX + 10, tooltipYPos, 10, weapon->isBroken() ? RED : GREEN);
            }
        } else if (hoveredItem->getType() == ItemType::ARMOR) {
            auto armor = std::dynamic_pointer_cast<GuardianArmor>(hoveredItem);
            if (armor) {
                DrawText(TextFormat("Armor: %d", armor->getStats().armor), tooltipX + 10, tooltipYPos, 10, BLUE);
                tooltipYPos += 15;
                DrawText(TextFormat("Durability: %.0f/%.0f", armor->getDurability(), armor->getMaxDurability()), 
                         tooltipX + 10, tooltipYPos, 10, armor->isBroken() ? RED : GREEN);
            }
        } else if (hoveredItem->getType() == ItemType::CONSUMABLE) {
            auto potion = std::dynamic_pointer_cast<AlchemicalPotion>(hoveredItem);
            if (potion) {
                auto effects = potion->getEffects();
                if (effects.health > 0) {
                    DrawText(TextFormat("Restores: %d HP", effects.health), tooltipX + 10, tooltipYPos, 10, GREEN);
                    tooltipYPos += 15;
                }
                if (effects.mana > 0) {
                    DrawText(TextFormat("Restores: %d MP", effects.mana), tooltipX + 10, tooltipYPos, 10, BLUE);
                }
            }
        }
        
        // Description
        if (!hoveredItem->getDescription().empty()) {
            DrawText("Description:", tooltipX + 10, tooltipY + tooltipHeight - 35, 10, YELLOW);
            DrawText(hoveredItem->getDescription().c_str(), tooltipX + 10, tooltipY + tooltipHeight - 20, 9, LIGHTGRAY);
        }
    }
    
    // Controls help - Simple and clear
    DrawText("Click: Equip/Use items | Hover: Item details | Press I to close", invX + 20, invY + invHeight - 25, 12, GREEN);
}

void renderPlayerStatsUI(const GameState& state) {
    // **TOP-LEFT PLAYER STATS** - Always visible
    int statsX = 10;
    int statsY = 180;  // Below existing UI
    int statsWidth = 200;
    int statsHeight = 120;
    
    DrawRectangle(statsX, statsY, statsWidth, statsHeight, Fade(DARKBLUE, 0.8f));
    DrawRectangleLines(statsX, statsY, statsWidth, statsHeight, SKYBLUE);
    
    DrawText("PLAYER STATUS", statsX + 10, statsY + 5, 14, SKYBLUE);
    
    // Health bar
    float healthPercent = (float)state.playerHealth / state.maxPlayerHealth;
    Color healthColor = healthPercent > 0.6f ? GREEN : (healthPercent > 0.3f ? YELLOW : RED);
    DrawText(TextFormat("HP: %d/%d", state.playerHealth, state.maxPlayerHealth), 
             statsX + 10, statsY + 25, 12, healthColor);
    DrawRectangle(statsX + 10, statsY + 40, 180, 8, DARKGRAY);
    DrawRectangle(statsX + 10, statsY + 40, (int)(180 * healthPercent), 8, healthColor);
    
    // Mana bar
    float manaPercent = (float)state.playerMana / state.maxPlayerMana;
    Color manaColor = manaPercent > 0.6f ? BLUE : (manaPercent > 0.3f ? SKYBLUE : PURPLE);
    DrawText(TextFormat("MP: %d/%d", state.playerMana, state.maxPlayerMana), 
             statsX + 10, statsY + 55, 12, manaColor);
    DrawRectangle(statsX + 10, statsY + 70, 180, 8, DARKGRAY);
    DrawRectangle(statsX + 10, statsY + 70, (int)(180 * manaPercent), 8, manaColor);
    
    // Level and experience
    DrawText(TextFormat("Level %d", state.playerLevel), statsX + 10, statsY + 85, 12, GOLD);
    DrawText(TextFormat("XP: %d", state.playerExperience), statsX + 100, statsY + 85, 12, YELLOW);
    
    // Equipment bonuses (if any)
    if (state.inventorySystem) {
        auto totalStats = state.inventorySystem->getTotalBonuses();
        if (totalStats.damage > 0 || totalStats.armor > 0) {
            DrawText(TextFormat("DMG+%d ARM+%d", totalStats.damage, totalStats.armor), 
                     statsX + 10, statsY + 100, 10, GREEN);
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
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
    
    // **CENTERED MENU WINDOW**
    int menuWidth = 400;
    int menuHeight = 300;
    int menuX = (screenWidth - menuWidth) / 2;
    int menuY = (screenHeight - menuHeight) / 2;
    
    // Menu background
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, DARKBLUE);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, SKYBLUE);
    
    // **TITLE**
    DrawText("= BROWSERWIND PAUSED =", menuX + 80, menuY + 20, 20, SKYBLUE);
    
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
        Color optionColor = isSelected ? YELLOW : WHITE;
        Color bgColor = isSelected ? Fade(YELLOW, 0.3f) : Fade(GRAY, 0.1f);
        
        // **ENHANCED HIGHLIGHT** for hovered/selected option
        if (isSelected) {
            DrawRectangle(menuX + 15, optionY + i * optionSpacing - 8, menuWidth - 30, 44, bgColor);
            DrawRectangleLines(menuX + 15, optionY + i * optionSpacing - 8, menuWidth - 30, 44, YELLOW);
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
        DrawText(hoverHints[hoveredOption], menuX + 30, menuY + menuHeight - 40, 12, YELLOW);
    } else {
        DrawText("Hover over options to see details | Click to select | ESC to resume", menuX + 30, menuY + menuHeight - 40, 12, LIGHTGRAY);
    }
}