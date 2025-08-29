// combat.h
#ifndef COMBAT_H
#define COMBAT_H

#include "raylib.h"
#include "game_state.h"

struct LongswordSwing {
    Vector3 startPosition;
    Vector3 endPosition;
    Vector3 direction;
    bool active;
    float progress;
    float lifetime;
};

struct Target {
    Vector3 position;
    bool active;
    bool hit;
    float hitTime;
    Color color;
};

extern const int MAX_SWINGS;
extern LongswordSwing swings[];
extern float lastSwingTime;
extern const float swingCooldown;
extern const float swingRange;
extern const float swingSpeed;
extern const float swingDuration;

extern const int MAX_TARGETS;
extern Target targets[];

void initCombat();
void updateMeleeSwing(Camera3D camera, float currentTime, GameState& state);
void updateSwings();
void updateTargets();
void renderCombat(Camera3D camera, float currentTime);

#endif