// render_utils.h
#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "raylib.h"
#include "environmental_object.h"
#include "npc.h"
#include "environment_manager.h"
#include "game_state.h"
#include <string>

void renderBuildingInterior(const Building& building);
void renderNPC(const NPC& npc, Camera3D camera, float currentTime);
void renderProjectedLabels(Camera3D camera, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding);
void renderUI(Camera3D camera, float currentTime, const GameState& state, bool testBuildingCollision);
void renderTestingPanel(const GameState& state, const std::string& locationText, Color locationColor);
void renderGameStats(const GameState& state);
void renderDialogWindow(const GameState& state);
void renderInventoryUI(const GameState& state);
void renderPlayerStatsUI(const GameState& state);
void renderEscMenu(GameState& state);
void printFinalSummary(const GameState& state);

#endif