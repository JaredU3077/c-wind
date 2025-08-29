// testing_system.h
#ifndef TESTING_SYSTEM_H
#define TESTING_SYSTEM_H

#include "raylib.h"
#include "game_state.h"

// Forward declaration to avoid circular dependency
class EnvironmentManager;

void handleTesting(Camera3D camera, const EnvironmentManager& environment, GameState& state);

#endif