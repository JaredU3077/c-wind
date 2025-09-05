#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "raylib.h"
#include "game_state.h"
#include "environment_manager.h"

/// \brief Updates the player's state including movement, jumping, and collisions.
/// \param camera The camera to update.
/// \param state The game state.
/// \param environment The environment manager.
/// \param deltaTime Time since last frame.
void updatePlayer(Camera3D& camera, GameState& state, const EnvironmentManager& environment, float deltaTime);

#endif