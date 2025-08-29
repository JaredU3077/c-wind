// player_system.h
#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "raylib.h"
#include "game_state.h"
#include "environment_manager.h"

void updatePlayer(Camera3D& camera, GameState& state, const EnvironmentManager& environment, float deltaTime);

#endif