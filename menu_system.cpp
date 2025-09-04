#include "menu_system.h"
#include <iostream>

MenuSystem::MenuSystem(GameState& state) : state_(state) {}

void MenuSystem::handleEscMenuInput() {
    if (state_.showEscMenu && !state_.enhancedInput.isMouseCaptured()) {
        handleEscMenuClick();
    }
}

void MenuSystem::handleInventoryInput(const std::unique_ptr<InventorySystem>& inventorySystem) {
    if (state_.showInventoryWindow && !state_.enhancedInput.isMouseCaptured()) {
        handleInventoryClick(inventorySystem);
    }
}

void MenuSystem::handleEscMenuClick() {
    Vector2 mousePos = GetMousePosition();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int menuWidth = 400;
    int menuHeight = 300;
    int menuX = (screenWidth - menuWidth) / 2;
    int menuY = (screenHeight - menuHeight) / 2;
    int optionY = menuY + 80;
    int optionSpacing = 50;

    if (state_.enhancedInput.isMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < 4; i++) {
            int buttonY = optionY + i * optionSpacing - 5;
            if (mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuWidth - 20 &&
                mousePos.y >= buttonY && mousePos.y <= buttonY + 40) {

                state_.selectedMenuOption = i;
                std::cout << "ESC Menu clicked: Option " << i << std::endl;

                switch (i) {
                    case 0: // Resume Game
                        state_.showEscMenu = false;
                        state_.enhancedInput.setMouseCaptured(true);
                        state_.mouseReleased = false;
                        std::cout << "Resuming game..." << std::endl;
                        break;

                    case 1: // Save Game
                        std::cout << "Saving game..." << std::endl;
                        state_.saveState();
                        std::cout << "Game saved successfully!" << std::endl;
                        break;

                    case 2: // Load Game
                        std::cout << "Loading game..." << std::endl;
                        if (state_.loadState()) {
                            std::cout << "Game loaded successfully!" << std::endl;
                        } else {
                            std::cout << "No save file found or load failed!" << std::endl;
                        }
                        break;

                    case 3: // Quit Game
                        std::cout << "Quitting game..." << std::endl;
                        state_.shouldClose = true;  // Assume added to GameState or access via Game
                        break;
                }
                break;
            }
        }
    }
}

void MenuSystem::handleInventoryClick(const std::unique_ptr<InventorySystem>& inventorySystem) {
    Vector2 mousePos = GetMousePosition();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int invWidth = 500;
    int invHeight = 400;
    int invX = (screenWidth - invWidth) / 2;
    int invY = (screenHeight - invHeight) / 2;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int itemListY = invY + 85;
        int itemHeight = 18;

        int itemIndex = (mousePos.y - itemListY) / itemHeight;
        if (itemIndex >= 0 && itemIndex < 12 && mousePos.x >= invX + 20 && mousePos.x <= invX + 470) {
            auto items = inventorySystem->getInventory().getAllItems();
            if (itemIndex < (int)items.size()) {
                auto clickedItem = items[itemIndex];
                state_.lastClickedItem = clickedItem->getName();

                std::cout << "Inventory Click: " << clickedItem->getName() << " ["
                         << ItemUtils::rarityToString(clickedItem->getRarity()) << "]" << std::endl;

                // **AUTO-EQUIP/USE LOGIC** - Moved here
                if (clickedItem->getType() == ItemType::WEAPON || clickedItem->getType() == ItemType::ARMOR) {
                    if (inventorySystem->equipItemFromInventory(clickedItem->getName())) {
                        std::cout << "Equipped: " << clickedItem->getName() << std::endl;
                    } else {
                        std::cout << "Cannot equip: " << clickedItem->getName() << std::endl;
                    }
                } else if (clickedItem->getType() == ItemType::CONSUMABLE) {
                    inventorySystem->getInventory().removeItem(clickedItem, 1);

                    auto potion = std::dynamic_pointer_cast<AlchemicalPotion>(clickedItem);
                    if (potion) {
                        auto effects = potion->getEffects();
                        state_.playerHealth = std::min(state_.maxPlayerHealth, state_.playerHealth + effects.health);
                        state_.playerMana = std::min(state_.maxPlayerMana, state_.playerMana + effects.mana);
                        state_.playerStamina = std::min(state_.maxPlayerStamina, state_.playerStamina + effects.stamina);
                        std::cout << "Used " << clickedItem->getName() << " - Applied effects: HP+" << effects.health << " MP+" << effects.mana << " SP+" << effects.stamina << std::endl;
                    }
                }
            }
        }
    }
}