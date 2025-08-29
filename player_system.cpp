// player_system.cpp
#include "player_system.h"
#include "collision_system.h"
#include "raylib.h"
#include "raymath.h"  // For Vector3Subtract, Vector3Add

void updatePlayer(Camera3D& camera, GameState& state, const EnvironmentManager& environment, float deltaTime) {
    const float gravity = -15.0f;
    const float jumpStrength = 8.0f;
    const float groundLevel = 0.0f;
    const float eyeHeight = 1.75f;
    const float PLAYER_RADIUS = 0.4f;
    const float PLAYER_HEIGHT = 1.8f;

    // Handle jumping
    if (!state.isInDialog && IsKeyPressed(KEY_SPACE) && state.isGrounded && !state.isJumping) {
        state.isJumping = true;
        state.isGrounded = false;
        state.jumpVelocity = jumpStrength;
        state.testSpaceJump = true;
    }

    if (state.isJumping || !state.isGrounded) {
        state.jumpVelocity += gravity * deltaTime;
        state.playerY += state.jumpVelocity * deltaTime;

        if (state.playerY <= groundLevel) {
            state.playerY = groundLevel;
            state.isJumping = false;
            state.isGrounded = true;
            state.jumpVelocity = 0.0f;
        }
    }

    camera.position.y = state.playerY + eyeHeight;

    // Only update camera when not in dialog mode
    if (!state.isInDialog) {
        // Store original position before any movement
        Vector3 originalPosition = camera.position;

        // Calculate intended new position and view from camera controls
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);  // Update camera (movement and looking)

        Vector3 intendedPosition = camera.position;
        Vector3 viewDirection = Vector3Subtract(camera.target, intendedPosition);

        // Reset camera to original position for collision checking
        camera.position = originalPosition;

        // Apply collision detection and resolution to intended position
        CollisionSystem::resolveCollisions(intendedPosition, originalPosition, PLAYER_RADIUS, PLAYER_HEIGHT, state.playerY, eyeHeight, groundLevel, environment, state.isInBuilding, state.currentBuilding);

        // Set adjusted position and preserve view direction
        camera.position = intendedPosition;
        camera.target = Vector3Add(camera.position, viewDirection);

        // Update playerY based on new camera y
        state.playerY = camera.position.y - eyeHeight;
    }
}