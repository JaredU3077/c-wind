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

/// \brief System for handling collisions.
class CollisionSystem {
public:
    /// \brief Checks collision between two bounds.
    /// \param bounds1 First bounds.
    /// \param bounds2 Second bounds.
    /// \return True if colliding.
    static bool checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2);

    /// \brief Checks if point is in bounds.
    /// \param point Point to check.
    /// \param bounds Bounds to check against.
    /// \return True if inside.
    static bool checkPointInBounds(Vector3 point, const CollisionBounds& bounds);

    /// \brief Converts bounds to bounding box.
    /// \param bounds Bounds to convert.
    /// \return Bounding box.
    static BoundingBox boundsToBox(const CollisionBounds& bounds);

    /// \brief Resolves collisions for player movement.
    /// \param newPosition Intended position.
    /// \param originalPosition Current position.
    /// \param playerRadius Player radius.
    /// \param playerHeight Player height.
    /// \param playerY Player Y.
    /// \param eyeHeight Eye height.
    /// \param groundLevel Ground level.
    /// \param environment Environment manager.
    /// \param isInBuilding If in building.
    /// \param currentBuilding Current building ID.
    static void resolveCollisions(Vector3& newPosition, const Vector3& originalPosition, float playerRadius, float playerHeight, float playerY, float eyeHeight, float groundLevel, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding);

    // Door-specific collision checking
    /// \brief Checks door collision.
    /// \param playerBounds Player bounds.
    /// \param environment Environment.
    /// \param doorBuildingId Output building ID.
    /// \return True if colliding with door.
    static bool checkDoorCollision(const CollisionBounds& playerBounds, const EnvironmentManager& environment, int& doorBuildingId);

    /// \brief Checks if can enter building.
    /// \param buildingId Building ID.
    /// \param environment Environment.
    /// \return True if can enter.
    static bool canEnterBuilding(int buildingId, const EnvironmentManager& environment);

private:
    static bool checkBoxCollision(const CollisionBounds& box1, const CollisionBounds& bounds2);
    static bool checkSphereCollision(const CollisionBounds& sphere1, const CollisionBounds& bounds2);
    static bool checkCylinderCollision(const CollisionBounds& cyl1, const CollisionBounds& bounds2);
    static bool checkCapsuleCollision(const CollisionBounds& capsule1, const CollisionBounds& bounds2);
    static bool checkCapsuleCollision(const CollisionBounds& capsule, const BoundingBox& box);
    static bool pointInBox(Vector3 point, const CollisionBounds& box);
    static bool pointInSphere(Vector3 point, const CollisionBounds& sphere);
    static bool pointInCylinder(Vector3 point, const CollisionBounds& cylinder);
    static bool pointInCapsule(Vector3 point, const CollisionBounds& capsule);
};

#endif