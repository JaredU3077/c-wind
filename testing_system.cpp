// testing_system.cpp
#include "testing_system.h"
#include "collision_system.h"

void handleTesting(Camera3D camera, const EnvironmentManager& environment, GameState& state) {
    if ((IsCursorHidden() && !state.mouseReleased) || state.isInDialog) {
        state.testMouseCaptured = true;
    }

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)) {
        state.testWASDMovement = true;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        state.testSpaceJump = true;
    }

    if (GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f) {
        state.testMouseLook = true;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state.testMeleeSwing = true;
    }

    if (camera.position.x != state.lastCameraPos.x ||
        camera.position.y != state.lastCameraPos.y ||
        camera.position.z != state.lastCameraPos.z) {
        state.testWASDMovement = true;
    }

    state.testBuildingCollision = false;
    Vector3 testPosition = camera.position;
    testPosition.x += 0.1f;
    int exclude = state.isInBuilding ? state.currentBuilding : -1;
    Vector3 boundsPos = testPosition;
    boundsPos.y = state.playerY;
    CollisionBounds playerBounds = {
        CollisionShape::CAPSULE,
        boundsPos,
        {0.4f, 1.8f, 0.0f},
        0.0f
    };
    if (environment.checkCollision(playerBounds, exclude)) {
        state.testBuildingCollision = true;
    }
}