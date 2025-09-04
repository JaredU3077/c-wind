// constants.h - Global constants for Browserwind game
#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================================================
// PLAYER CONSTANTS
// ============================================================================

namespace PlayerConstants {
    constexpr float EYE_HEIGHT = 1.75f;           // Camera height above ground
    constexpr float RADIUS = 0.4f;                // Player collision radius
    constexpr float HEIGHT = 1.8f;                // Player height
    constexpr float MOVE_SPEED = 5.0f;            // Base movement speed
    constexpr float JUMP_STRENGTH = 8.0f;        // Jump velocity
    constexpr float GRAVITY = -15.0f;            // Gravity acceleration
    constexpr float GROUND_LEVEL = 0.0f;         // Ground Y position
    constexpr float MOUSE_SENSITIVITY = 0.003f;   // Mouse look sensitivity
    constexpr float MAX_PITCH = 1.5f;             // Maximum look up/down angle
}

// ============================================================================
// ENVIRONMENT CONSTANTS
// ============================================================================

namespace EnvironmentConstants {
    constexpr float INTERACTION_DISTANCE = 3.0f;  // Distance for interactions
    constexpr float BUILDING_DOOR_HEIGHT = 2.0f;  // Standard door height
    constexpr float BUILDING_DOOR_WIDTH = 1.2f;   // Standard door width
    constexpr float TREE_TRUNK_HEIGHT = 4.0f;     // Standard tree height
    constexpr float TREE_TRUNK_RADIUS = 0.5f;     // Standard tree trunk radius
    constexpr float TREE_FOLIAGE_RADIUS = 2.5f;   // Standard tree foliage radius
    constexpr float WELL_BASE_RADIUS = 1.5f;      // Standard well radius
    constexpr float WELL_HEIGHT = 2.5f;           // Standard well height
}

// ============================================================================
// UI CONSTANTS
// ============================================================================

namespace UIConstants {
    constexpr int SCREEN_WIDTH_DEFAULT = 800;
    constexpr int SCREEN_HEIGHT_DEFAULT = 600;
    constexpr float UI_ALPHA_BACKGROUND = 0.95f;
    constexpr float UI_ALPHA_OVERLAY = 0.85f;
    constexpr float UI_ALPHA_TOOLTIPS = 0.98f;
    constexpr float PULSE_SPEED = 5.0f;           // UI pulse animation speed
    constexpr float NPC_PULSE_SPEED = 3.0f;       // NPC interaction pulse speed
}

// ============================================================================
// GAME CONSTANTS
// ============================================================================

namespace GameConstants {
    constexpr float TARGET_FPS = 60.0f;
    constexpr float FRAME_TIME_60FPS = 1.0f / TARGET_FPS;
    constexpr int MAX_SWINGS = 3;
    constexpr float SWING_COOLDOWN = 0.5f;
    constexpr float SWING_RANGE = 3.0f;
    constexpr float SWING_SPEED = 8.0f;
    constexpr float SWING_DURATION = 0.3f;
    constexpr int MAX_TARGETS = 5;
    constexpr int MAX_NPCS = 2;
    constexpr int INVENTORY_CAPACITY = 150.0f;     // kg
    constexpr int INVENTORY_SLOTS = 60;
}

// ============================================================================
// RENDERING CONSTANTS
// ============================================================================

namespace RenderConstants {
    constexpr int CYLINDER_SEGMENTS = 16;         // Default cylinder resolution
    constexpr int TREE_CYLINDER_SEGMENTS = 8;     // Tree trunk resolution
    constexpr float BUILDING_ROOF_OFFSET = 0.5f;  // Roof height above building
    constexpr float FLOOR_OFFSET = 0.02f;         // Small offset for floor rendering
}

#endif // CONSTANTS_H
