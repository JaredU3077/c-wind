#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include "raylib.h"
#include "game_state.h"  // For GameState

class MenuSystem {
public:
    MenuSystem(GameState& state);
    void handleEscMenuInput();
    void handleInventoryInput(const std::unique_ptr<InventorySystem>& inventorySystem);

private:
    GameState& state_;
    void handleEscMenuClick();
    void handleInventoryClick(const std::unique_ptr<InventorySystem>& inventorySystem);
};

#endif