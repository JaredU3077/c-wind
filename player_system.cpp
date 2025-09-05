#include "player_system.h"
#include "collision_system.h"
#include "constants.h"
#include "raylib.h"
#include "raymath.h"  // For Vector3Subtract, Vector3Add, Vector3Normalize, Vector3CrossProduct, Vector3Scale
#include "math_utils.h"  // For MathUtils::distance3D
#include <iostream>  // For std::cout debug output

void updatePlayer(Camera3D& camera, GameState& state, const EnvironmentManager& environment, float deltaTime) {
    const float gravity = PlayerConstants::GRAVITY;
    const float jump_strength = PlayerConstants::JUMP_STRENGTH;
    const float ground_level = PlayerConstants::GROUND_LEVEL;
    const float eye_height = PlayerConstants::EYE_HEIGHT;
    const float player_radius = PlayerConstants::RADIUS;
    const float player_height = PlayerConstants::HEIGHT;

    // **JUMPING LOGIC** - Enhanced with input check and state management
    if (!state.isInDialog && !state.showInventoryWindow && !state.showEscMenu && 
        state.enhancedInput.isActionPressed("jump") && state.isGrounded && !state.isJumping) {
        state.isJumping = true;
        state.isGrounded = false;
        state.jumpVelocity = jump_strength;
        state.testSpaceJump = true;
        std::cout << "Jump initiated: velocity=" << state.jumpVelocity << std::endl;
    }

    if (state.isJumping || !state.isGrounded) {
        state.jumpVelocity += gravity * deltaTime;
        state.playerY += state.jumpVelocity * deltaTime;

        if (state.playerY <= ground_level) {
            state.playerY = ground_level;
            state.isJumping = false;
            state.isGrounded = true;
            state.jumpVelocity = 0.0f;
            std::cout << "Landed: y=" << state.playerY << std::endl;
        }
    }

    camera.position.y = state.playerY + eye_height;

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
        Vector2 mouseDelta = state.enhancedInput.getMouseDelta();
        const float sensitivity = PlayerConstants::MOUSE_SENSITIVITY;

        // Debug what we get from enhanced input
        static int inputDebugCounter = 0;
        if (inputDebugCounter++ % 60 == 0) {
            std::cout << "PLAYER: Got mouse delta from input (" << mouseDelta.x << "," << mouseDelta.y << ")" << std::endl;
        }

        // Update camera angles for smooth rotation
        static float yaw = 0.0f;
        static float pitch = 0.0f;

        // Debug mouse input application - more frequent
        static int mouseDebugCounter = 0;
        if (mouseDebugCounter++ % 60 == 0) {
            std::cout << "CAMERA: Mouse delta (" << mouseDelta.x << ", " << mouseDelta.y
                      << ") Yaw: " << yaw << " Pitch: " << pitch << std::endl;
        }

        yaw -= mouseDelta.x * sensitivity;
        pitch -= mouseDelta.y * sensitivity;

        // Clamp pitch to prevent over-rotation
        pitch = std::clamp(pitch, -PlayerConstants::MAX_PITCH, PlayerConstants::MAX_PITCH);
        
        // Calculate new look direction
        Vector3 direction = {
            std::cos(pitch) * std::sin(yaw),
            std::sin(pitch),
            std::cos(pitch) * std::cos(yaw)
        };
        camera.target = Vector3Add(originalPosition, direction);

        // Debug camera target
        static int cameraTargetCounter = 0;
        if (cameraTargetCounter++ % 120 == 0) {
            std::cout << "CAMERA TARGET: (" << camera.target.x << ", " << camera.target.y << ", " << camera.target.z << ")" << std::endl;
        }
        
        // **LINEAR MOVEMENT**: Calculate precise movement vectors
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, {0.0f, 1.0f, 0.0f}));
        
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
        if (state.enhancedInput.isActionDown("move_forward")) {
            movement = Vector3Add(movement, Vector3Scale(forward, moveSpeed));
        }
        if (state.enhancedInput.isActionDown("move_backward")) {
            movement = Vector3Add(movement, Vector3Scale(forward, -moveSpeed));
        }
        if (state.enhancedInput.isActionDown("strafe_right")) {  // Strafe right - perfectly linear
            movement = Vector3Add(movement, Vector3Scale(right, moveSpeed));
        }
        if (state.enhancedInput.isActionDown("strafe_left")) {  // Strafe left - perfectly linear
            movement = Vector3Add(movement, Vector3Scale(right, -moveSpeed));
        }
        
        // Calculate intended position
        Vector3 intendedPosition = Vector3Add(originalPosition, movement);

        // Debug: Log movement if significant
        Vector3 totalMovement = Vector3Subtract(intendedPosition, originalPosition);
        if (Vector3Length(totalMovement) > 0.01f) {
            static int moveCounter = 0;
            if (++moveCounter % 30 == 0) {
                std::cout << "Player movement: (" << totalMovement.x << ", " << totalMovement.y << ", " << totalMovement.z << ")" << std::endl;
            }
        }

        // Apply collision detection and resolution
        CollisionSystem::resolveCollisions(intendedPosition, originalPosition, player_radius, player_height, state.playerY, eye_height, ground_level, environment, state.isInBuilding, state.currentBuilding);

        // Set final camera position after collision resolution
        camera.position = intendedPosition;
        // **FIXED**: Update camera target to maintain look direction from new position
        Vector3 viewDirection = Vector3Subtract(camera.target, originalPosition);
        camera.target = Vector3Add(camera.position, viewDirection);

        // Update playerY based on new camera y
        state.playerY = camera.position.y - eye_height;

        // Debug: Log final position changes
        static Vector3 lastPosition = {0, 0, 0};
        if (MathUtils::distance3D(camera.position, lastPosition) > 0.1f) {
            static int posCounter = 0;
            if (++posCounter % 30 == 0) {
                std::cout << "Final position: (" << camera.position.x << ", " << camera.position.y << ", " << camera.position.z << ") | In building: " << state.isInBuilding << std::endl;
            }
            lastPosition = camera.position;
        }
    }
}