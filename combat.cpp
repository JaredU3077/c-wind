// combat.cpp
#include "combat.h"
#include "math_utils.h"
#include "constants.h"
#include <cmath>

const int MAX_SWINGS = GameConstants::MAX_SWINGS;
LongswordSwing swings[MAX_SWINGS];
float lastSwingTime = 0.0f;
const float swingCooldown = GameConstants::SWING_COOLDOWN;
const float swingRange = GameConstants::SWING_RANGE;
const float swingSpeed = GameConstants::SWING_SPEED;
const float swingDuration = GameConstants::SWING_DURATION;

const int MAX_TARGETS = GameConstants::MAX_TARGETS;
Target targets[MAX_TARGETS];

void initCombat() {
    for (int i = 0; i < MAX_SWINGS; i++) {
        swings[i].active = false;
    }

    Vector3 targetPositions[MAX_TARGETS] = {
        {-8.0f, 3.0f, -5.0f},
        {8.0f, 3.0f, -5.0f},
        {0.0f, 3.0f, -10.0f},
        {-5.0f, 4.0f, 5.0f},
        {5.0f, 4.0f, 5.0f}
    };
    Color targetColors[MAX_TARGETS] = {RED, GREEN, BLUE, YELLOW, PURPLE};

    for (int i = 0; i < MAX_TARGETS; i++) {
        targets[i].position = targetPositions[i];
        targets[i].active = true;
        targets[i].hit = false;
        targets[i].color = targetColors[i];
    }
}

void updateMeleeSwing(Camera3D camera, float currentTime, GameState& state) {
    for (int i = 0; i < MAX_SWINGS; i++) {
        if (!swings[i].active) {
            swings[i].startPosition = camera.position;
            swings[i].startPosition.y -= 0.5f;

            Vector3 forward = {
                camera.target.x - camera.position.x,
                camera.target.y - camera.position.y,
                camera.target.z - camera.position.z
            };

            // Normalize the forward vector using math utilities
            forward = MathUtils::normalizeVector3D(forward);

            swings[i].direction = forward;
            swings[i].endPosition = {
                swings[i].startPosition.x + forward.x * swingRange,
                swings[i].startPosition.y + forward.y * swingRange,
                swings[i].startPosition.z + forward.z * swingRange
            };

            swings[i].active = true;
            swings[i].progress = 0.0f;
            swings[i].lifetime = swingDuration;
            lastSwingTime = currentTime;
            state.swingsPerformed++;
            state.testMeleeSwing = true;

            for (int t = 0; t < MAX_TARGETS; t++) {
                if (targets[t].active && !targets[t].hit) {
                    // Calculate distance to target using math utilities
                    float distance = MathUtils::distance3D(targets[t].position, swings[i].startPosition);

                    if (distance <= swingRange) {
                        targets[t].hit = true;
                        targets[t].hitTime = GetTime();
                        state.meleeHits++;
                        state.score += 150;
                        state.testMeleeHitDetection = true;
                    }
                }
            }
            break;
        }
    }
}

void updateSwings() {
    for (int i = 0; i < MAX_SWINGS; i++) {
        if (swings[i].active) {
            swings[i].progress += swingSpeed * GetFrameTime();
            swings[i].lifetime -= GetFrameTime();

            if (swings[i].progress >= 1.0f || swings[i].lifetime <= 0) {
                swings[i].active = false;
            }
        }
    }
}

void updateTargets() {
    for (int t = 0; t < MAX_TARGETS; t++) {
        if (targets[t].hit && (GetTime() - targets[t].hitTime) > 2.0f) {
            targets[t].hit = false;
            targets[t].active = true;
        }
    }
}

void renderCombat([[maybe_unused]] Camera3D camera, [[maybe_unused]] float currentTime) {
    for (int s = 0; s < MAX_SWINGS; s++) {
        if (swings[s].active) {
            Vector3 currentPos = {
                swings[s].startPosition.x + swings[s].direction.x * swingRange * swings[s].progress,
                swings[s].startPosition.y + swings[s].direction.y * swingRange * swings[s].progress,
                swings[s].startPosition.z + swings[s].direction.z * swingRange * swings[s].progress
            };

            DrawCylinderEx(swings[s].startPosition, currentPos, 0.02f, 0.01f, 8, RED);

            Vector3 handleStart = swings[s].startPosition;
            Vector3 handleEnd = {
                handleStart.x + swings[s].direction.x * 0.5f,
                handleStart.y + swings[s].direction.y * 0.5f,
                handleStart.z + swings[s].direction.z * 0.5f
            };
            DrawCylinderEx(handleStart, handleEnd, 0.03f, 0.02f, 8, DARKBROWN);

            DrawSphere(currentPos, 0.1f, Fade(YELLOW, 0.7f));

            for (float trail = 0.2f; trail <= swings[s].progress; trail += 0.2f) {
                Vector3 trailPos = {
                    swings[s].startPosition.x + swings[s].direction.x * swingRange * trail,
                    swings[s].startPosition.y + swings[s].direction.y * swingRange * trail,
                    swings[s].startPosition.z + swings[s].direction.z * swingRange * trail
                };
                DrawSphere(trailPos, 0.03f, Fade(ORANGE, 0.3f * (1.0f - trail)));
            }
        }
    }

    bool isSwinging = false;
    for (int s = 0; s < MAX_SWINGS; s++) {
        if (swings[s].active) {
            isSwinging = true;
            break;
        }
    }

    if (isSwinging) {
        DrawSphereWires(camera.position, swingRange, 8, 16, Fade(RED, 0.5f));
    }
}