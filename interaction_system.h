// interaction_system.h
#ifndef INTERACTION_SYSTEM_H
#define INTERACTION_SYSTEM_H

#include "raylib.h"
#include "environment_manager.h"
#include "npc.h"
#include "dialog_system.h"
#include "game_state.h"

void handleInteractions(Camera3D& camera, EnvironmentManager& environment, GameState& state, float currentTime);

#endif