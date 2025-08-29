// dialog_system.h
#ifndef DIALOG_SYSTEM_H
#define DIALOG_SYSTEM_H

#include "game_state.h"

void startDialog(int npcIndex, GameState& state);
void handleDialogOption(int optionIndex, GameState& state);

#endif