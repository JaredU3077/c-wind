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

    // Handle jumping - disabled during dialog, inventory, or ESC menu
    if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu && IsKeyPressed(KEY_SPACE) && state.isGrounded && !state.isJumping) {
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

    // Only update camera when not in dialog mode, inventory closed, and ESC menu closed
    if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu) {
        // **MANUAL MOVEMENT**: Calculate precise linear movement vectors to fix strafing curvature
        Vector3 originalPosition = camera.position;
        Vector3 originalTarget = camera.target;
        
        // **MOUSE LOOK**: Handle camera rotation first
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || true) {  // Always allow mouse look in FPS
            Vector2 mouseDelta = GetMouseDelta();
            const float sensitivity = 0.003f;
            
            // Update camera angles for smooth rotation
            static float yaw = 0.0f;
            static float pitch = 0.0f;
            
            yaw -= mouseDelta.x * sensitivity;
            pitch -= mouseDelta.y * sensitivity;
            
            // Clamp pitch to prevent over-rotation
            if (pitch > 1.5f) pitch = 1.5f;
            if (pitch < -1.5f) pitch = -1.5f;
            
            // Calculate new look direction
            Vector3 direction = {
                cosf(pitch) * sinf(yaw),
                sinf(pitch),
                cosf(pitch) * cosf(yaw)
            };
            camera.target = Vector3Add(originalPosition, direction);
        }
        
        // **LINEAR MOVEMENT**: Calculate precise movement vectors
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0.0f, 1.0f, 0.0f}));
        
        // **CRITICAL FIX**: Ensure perfectly horizontal movement for linear strafing
        forward.y = 0.0f;
        forward = Vector3Normalize(forward);
        right.y = 0.0f;
        right = Vector3Normalize(right);
        
        // Movement speed
        const float moveSpeed = 5.0f * deltaTime;
        
        // **PRECISE MOVEMENT**: Calculate movement vector with no curvature
        Vector3 movement = {0.0f, 0.0f, 0.0f};
        
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            movement = Vector3Add(movement, Vector3Scale(forward, moveSpeed));
        }
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
            movement = Vector3Add(movement, Vector3Scale(forward, -moveSpeed));
        }
        if (IsKeyDown(KEY_D)) {  // Strafe right - perfectly linear
            movement = Vector3Add(movement, Vector3Scale(right, moveSpeed));
        }
        if (IsKeyDown(KEY_A)) {  // Strafe left - perfectly linear
            movement = Vector3Add(movement, Vector3Scale(right, -moveSpeed));
        }
        
        // Calculate intended position
        Vector3 intendedPosition = Vector3Add(originalPosition, movement);

        // Debug: Log movement if significant
        Vector3 totalMovement = Vector3Subtract(intendedPosition, originalPosition);
        if (Vector3Length(totalMovement) > 0.01f) {
            static int moveCounter = 0;
            moveCounter++;
            if (moveCounter % 30 == 0) {  // Log every 30 movements
                printf("Player movement: (%.3f, %.3f, %.3f) -> (%.3f, %.3f, %.3f)\n",
                       originalPosition.x, originalPosition.y, originalPosition.z,
                       intendedPosition.x, intendedPosition.y, intendedPosition.z);
            }
        }

        // Apply collision detection and resolution
        CollisionSystem::resolveCollisions(intendedPosition, originalPosition, PLAYER_RADIUS, PLAYER_HEIGHT, state.playerY, eyeHeight, groundLevel, environment, state.isInBuilding, state.currentBuilding);

        // Set final camera position after collision resolution
        camera.position = intendedPosition;
        // **FIXED**: Update camera target to maintain look direction from new position
        Vector3 viewDirection = Vector3Subtract(camera.target, originalPosition);
        camera.target = Vector3Add(camera.position, viewDirection);

        // Update playerY based on new camera y
        state.playerY = camera.position.y - eyeHeight;

        // Debug: Log final position changes
        static Vector3 lastPosition = {0, 0, 0};
        if (Vector3Distance(camera.position, lastPosition) > 0.1f) {
            static int posCounter = 0;
            posCounter++;
            if (posCounter % 30 == 0) {
                printf("Final position: (%.1f, %.1f, %.1f) | In building: %d\n",
                       camera.position.x, camera.position.y, camera.position.z, state.isInBuilding);
            }
            lastPosition = camera.position;
        }
    }
}