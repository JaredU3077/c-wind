// player_system.cpp
#include "player_system.h"
#include "collision_system.h"
#include "constants.h"
#include "raylib.h"
#include "raymath.h"  // For Vector3Subtract, Vector3Add
#include "math_utils.h"  // For MathUtils::distance3D
#include <iostream>  // For std::cout debug output

void updatePlayer(Camera3D& camera, GameState& state, const EnvironmentManager& environment, float deltaTime) {
    const float gravity = PlayerConstants::GRAVITY;
    const float jumpStrength = PlayerConstants::JUMP_STRENGTH;
    const float groundLevel = PlayerConstants::GROUND_LEVEL;
    const float eyeHeight = PlayerConstants::EYE_HEIGHT;
    const float PLAYER_RADIUS = PlayerConstants::RADIUS;
    const float PLAYER_HEIGHT = PlayerConstants::HEIGHT;

    // **PHASE 2 ENHANCEMENT**: Handle jumping using enhanced input with action binding
    if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu && 
        state.enhancedInput.isActionPressed("jump") && state.isGrounded && !state.isJumping) {
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
        // Debug: Log movement state
        static int movementDebugCounter = 0;
        if (movementDebugCounter++ % 120 == 0) {  // Log every 2 seconds at 60 FPS
            std::cout << "MOVEMENT ENABLED: Dialog=" << state.isInDialog
                      << ", Inventory=" << state.showInventoryWindow
                      << ", EscMenu=" << state.showEscMenu << std::endl;
        }
        // **MANUAL MOVEMENT**: Calculate precise linear movement vectors to fix strafing curvature
        Vector3 originalPosition = camera.position;
        
        // **MOUSE LOOK**: Handle camera rotation first
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || true) {  // Always allow mouse look in FPS
            Vector2 mouseDelta = GetMouseDelta();
            const float sensitivity = PlayerConstants::MOUSE_SENSITIVITY;

            // Update camera angles for smooth rotation
            static float yaw = 0.0f;
            static float pitch = 0.0f;

            yaw -= mouseDelta.x * sensitivity;
            pitch -= mouseDelta.y * sensitivity;

            // Clamp pitch to prevent over-rotation
            if (pitch > PlayerConstants::MAX_PITCH) pitch = PlayerConstants::MAX_PITCH;
            if (pitch < -PlayerConstants::MAX_PITCH) pitch = -PlayerConstants::MAX_PITCH;
            
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
        const float moveSpeed = PlayerConstants::MOVE_SPEED * deltaTime;
        
        // **PRECISE MOVEMENT**: Calculate movement vector with no curvature
        Vector3 movement = {0.0f, 0.0f, 0.0f};
        
        // **PHASE 2 ENHANCEMENT**: Movement input using enhanced input manager with action bindings
        bool wPressed = state.enhancedInput.isActionDown("move_forward") || IsKeyDown(KEY_W);
        bool sPressed = state.enhancedInput.isActionDown("move_backward") || IsKeyDown(KEY_S);
        bool dPressed = state.enhancedInput.isActionDown("strafe_right") || IsKeyDown(KEY_D);
        bool aPressed = state.enhancedInput.isActionDown("strafe_left") || IsKeyDown(KEY_A);

        // Debug: Log key presses occasionally
        static int keyDebugCounter = 0;
        if (keyDebugCounter++ % 300 == 0) {  // Log every 5 seconds at 60 FPS
            std::cout << "WASD Input: W=" << wPressed << ", S=" << sPressed
                      << ", A=" << aPressed << ", D=" << dPressed << std::endl;

            // Debug key bindings
            int wKey = state.enhancedInput.getKeyBinding("move_forward");
            int sKey = state.enhancedInput.getKeyBinding("move_backward");
            std::cout << "Key Bindings: move_forward=" << wKey << " (KEY_W=" << KEY_W << "), move_backward=" << sKey << " (KEY_S=" << KEY_S << ")" << std::endl;

            // Debug raw raylib key detection
            bool wRaw = IsKeyDown(KEY_W);
            bool sRaw = IsKeyDown(KEY_S);
            bool aRaw = IsKeyDown(KEY_A);
            bool dRaw = IsKeyDown(KEY_D);
            std::cout << "Raw Raylib Keys: W=" << wRaw << ", S=" << sRaw << ", A=" << aRaw << ", D=" << dRaw << std::endl;

            // Debug enhanced input manager state
            bool wEnhanced = state.enhancedInput.isActionDown("move_forward");
            bool wKeyDown = state.enhancedInput.isKeyDown(KEY_W);
            std::cout << "Enhanced Input: isActionDown(move_forward)=" << wEnhanced << ", isKeyDown(KEY_W)=" << wKeyDown << std::endl;
        }

        if (wPressed) {
            movement = Vector3Add(movement, Vector3Scale(forward, moveSpeed));
        }
        if (sPressed) {
            movement = Vector3Add(movement, Vector3Scale(forward, -moveSpeed));
        }
        if (dPressed) {  // Strafe right - perfectly linear
            movement = Vector3Add(movement, Vector3Scale(right, moveSpeed));
        }
        if (aPressed) {  // Strafe left - perfectly linear
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
                #ifdef BROWSERWIND_DEBUG
                static int movementDebugCounter = 0;
                if (movementDebugCounter++ % 1800 == 0) {  // Once every 30 seconds
                    printf("Player movement: (%.3f, %.3f, %.3f) -> (%.3f, %.3f, %.3f)\n",
                       originalPosition.x, originalPosition.y, originalPosition.z,
                       intendedPosition.x, intendedPosition.y, intendedPosition.z);
                }
                #endif
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
        if (MathUtils::distance3D(camera.position, lastPosition) > 0.1f) {
            static int posCounter = 0;
            posCounter++;
            if (posCounter % 30 == 0) {
                #ifdef BROWSERWIND_DEBUG
                static int positionDebugCounter = 0;
                if (positionDebugCounter++ % 1800 == 0) {  // Once every 30 seconds
                    printf("Final position: (%.1f, %.1f, %.1f) | In building: %d\n",
                       camera.position.x, camera.position.y, camera.position.z, state.isInBuilding);
                }
                #endif
            }
            lastPosition = camera.position;
        }
    }
    } // Close the main if statement for dialog/inventory/esc menu check
}