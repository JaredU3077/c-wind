// collision_system.h
#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "raylib.h"
#include "npc.h"  // For checkNPCCollision
#include <algorithm>  // For std::clamp
#include <cmath>

// Forward declaration to avoid circular dependency
class EnvironmentManager;

// Collision system constants
constexpr float WALL_SLIDE_MULTIPLIER = 0.7f;
constexpr float MIN_MOVEMENT_THRESHOLD = 0.1f;
constexpr float DOOR_INTERACTION_DISTANCE = 3.0f;

enum class CollisionShape {
    BOX,
    SPHERE,
    CYLINDER,
    CAPSULE
};

struct CollisionBounds {
    CollisionShape shape;
    Vector3 position;
    Vector3 size;
    float rotation;
};

class CollisionSystem {
public:
    static bool checkCollision([[maybe_unused]] const CollisionBounds& bounds1, [[maybe_unused]] const CollisionBounds& bounds2);
    static bool checkPointInBounds([[maybe_unused]] Vector3 point, [[maybe_unused]] const CollisionBounds& bounds);
    static BoundingBox boundsToBox([[maybe_unused]] const CollisionBounds& bounds);

    static void resolveCollisions([[maybe_unused]] Vector3& newPosition, [[maybe_unused]] const Vector3& originalPosition, [[maybe_unused]] float playerRadius, [[maybe_unused]] float playerHeight, [[maybe_unused]] float playerY, [[maybe_unused]] float eyeHeight, [[maybe_unused]] float groundLevel, [[maybe_unused]] const EnvironmentManager& environment, [[maybe_unused]] bool isInBuilding, [[maybe_unused]] int currentBuilding);

    // Door-specific collision checking
    static bool checkDoorCollision([[maybe_unused]] const CollisionBounds& playerBounds, [[maybe_unused]] const EnvironmentManager& environment, [[maybe_unused]] int& doorBuildingId);
    static bool canEnterBuilding([[maybe_unused]] int buildingId, [[maybe_unused]] const EnvironmentManager& environment);

private:
    static bool checkBoxCollision([[maybe_unused]] const CollisionBounds& box1, [[maybe_unused]] const CollisionBounds& bounds2);
    static bool checkSphereCollision([[maybe_unused]] const CollisionBounds& sphere1, [[maybe_unused]] const CollisionBounds& bounds2);
    static bool checkCylinderCollision([[maybe_unused]] const CollisionBounds& cyl1, [[maybe_unused]] const CollisionBounds& bounds2);
    static bool checkCapsuleCollision([[maybe_unused]] const CollisionBounds& capsule1, [[maybe_unused]] const CollisionBounds& bounds2);
    static bool pointInBox([[maybe_unused]] Vector3 point, [[maybe_unused]] const CollisionBounds& box);
    static bool pointInSphere([[maybe_unused]] Vector3 point, [[maybe_unused]] const CollisionBounds& sphere);
    static bool pointInCylinder([[maybe_unused]] Vector3 point, [[maybe_unused]] const CollisionBounds& cylinder);
    static bool pointInCapsule([[maybe_unused]] Vector3 point, [[maybe_unused]] const CollisionBounds& capsule);
};

#endif