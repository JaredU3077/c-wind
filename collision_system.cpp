#include "collision_system.h"
#include "environment_manager.h"  // Now included for full use
#include "math_utils.h"  // For MathUtils::distance3D
#include "raymath.h"     // For Vector3Subtract, Vector3Length, etc.
#include <iostream>      // For debugging

// Implementation for checkPointInBounds
bool CollisionSystem::checkPointInBounds(Vector3 point, const CollisionBounds& bounds) {
    switch (bounds.shape) {
        case CollisionShape::BOX:
            return pointInBox(point, bounds);
        case CollisionShape::SPHERE:
            return pointInSphere(point, bounds);
        case CollisionShape::CYLINDER:
            return pointInCylinder(point, bounds);
        case CollisionShape::CAPSULE:
            return pointInCapsule(point, bounds);
    }
    return false;
}

bool CollisionSystem::pointInBox(Vector3 point, const CollisionBounds& box) {
    Vector3 min = {box.position.x - box.size.x / 2, box.position.y - box.size.y / 2, box.position.z - box.size.z / 2};
    Vector3 max = {box.position.x + box.size.x / 2, box.position.y + box.size.y / 2, box.position.z + box.size.z / 2};
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

bool CollisionSystem::pointInSphere(Vector3 point, const CollisionBounds& sphere) {
    float distance = MathUtils::distance3D(point, sphere.position);
    return distance <= sphere.size.x;  // Assume size.x is radius
}

bool CollisionSystem::pointInCylinder(Vector3 point, const CollisionBounds& cylinder) {
    // Project point onto cylinder axis (assume Y axis for simplicity)
    float proj = point.y - cylinder.position.y;
    if (std::abs(proj) > cylinder.size.y / 2) return false;  // height check
    float dist = std::sqrt(std::pow(point.x - cylinder.position.x, 2) + std::pow(point.z - cylinder.position.z, 2));
    return dist <= cylinder.size.x;  // radius check
}

bool CollisionSystem::pointInCapsule(Vector3 point, const CollisionBounds& capsule) {
    // Simplified capsule as two spheres + cylinder
    Vector3 start = capsule.position;
    Vector3 end = {capsule.position.x, capsule.position.y + capsule.size.y, capsule.position.z};
    float radius = capsule.size.x;

    // Closest point on line segment
    Vector3 dir = Vector3Subtract(end, start);
    float len = Vector3Length(dir);
    dir = Vector3Normalize(dir);
    float t = Vector3DotProduct(Vector3Subtract(point, start), dir);
    t = std::clamp(t, 0.0f, len);
    Vector3 closest = Vector3Add(start, Vector3Scale(dir, t));

    return Vector3Length(Vector3Subtract(point, closest)) <= radius;
}

// Implementation for boundsToBox
BoundingBox CollisionSystem::boundsToBox(const CollisionBounds& bounds) {
    Vector3 min = {bounds.position.x - bounds.size.x / 2, bounds.position.y - bounds.size.y / 2, bounds.position.z - bounds.size.z / 2};
    Vector3 max = {bounds.position.x + bounds.size.x / 2, bounds.position.y + bounds.size.y / 2, bounds.position.z + bounds.size.z / 2};
    return {min, max};
}

// Implementation for checkCollision
bool CollisionSystem::checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2) {
    if (bounds1.shape == CollisionShape::BOX && bounds2.shape == CollisionShape::BOX) {
        return checkBoxCollision(bounds1, bounds2);
    } else if (bounds1.shape == CollisionShape::SPHERE && bounds2.shape == CollisionShape::SPHERE) {
        return checkSphereCollision(bounds1, bounds2);
    } else if (bounds1.shape == CollisionShape::CYLINDER && bounds2.shape == CollisionShape::CYLINDER) {
        return checkCylinderCollision(bounds1, bounds2);
    } else if (bounds1.shape == CollisionShape::CAPSULE && bounds2.shape == CollisionShape::CAPSULE) {
        return checkCapsuleCollision(bounds1, bounds2);
    }
    // Fallback to box approximation for mixed shapes
    BoundingBox box1 = boundsToBox(bounds1);
    BoundingBox box2 = boundsToBox(bounds2);
    return CheckCollisionBoxes(box1, box2);
}

// Private collision checks
bool CollisionSystem::checkBoxCollision(const CollisionBounds& box1, const CollisionBounds& bounds2) {
    BoundingBox bb1 = boundsToBox(box1);
    BoundingBox bb2 = boundsToBox(bounds2);
    return CheckCollisionBoxes(bb1, bb2);
}

bool CollisionSystem::checkSphereCollision(const CollisionBounds& sphere1, const CollisionBounds& bounds2) {
    // Approximate bounds2 as sphere
    float dist = Vector3Distance(sphere1.position, bounds2.position);
    return dist <= (sphere1.size.x + bounds2.size.x);  // radii sum
}

bool CollisionSystem::checkCylinderCollision(const CollisionBounds& cyl1, const CollisionBounds& bounds2) {
    // Simplified: treat as cylinders along Y
    float distXZ = std::sqrt(std::pow(cyl1.position.x - bounds2.position.x, 2) + std::pow(cyl1.position.z - bounds2.position.z, 2));
    bool xzCollide = distXZ <= (cyl1.size.x + bounds2.size.x);
    bool yCollide = std::abs(cyl1.position.y - bounds2.position.y) <= (cyl1.size.y + bounds2.size.y) / 2;
    return xzCollide && yCollide;
}

bool CollisionSystem::checkCapsuleCollision(const CollisionBounds& capsule1, const CollisionBounds& bounds2) {
    // Convert bounds2 to BoundingBox and call the other overload
    BoundingBox box = {
        {bounds2.position.x - bounds2.size.x/2, bounds2.position.y - bounds2.size.y/2, bounds2.position.z - bounds2.size.z/2},
        {bounds2.position.x + bounds2.size.x/2, bounds2.position.y + bounds2.size.y/2, bounds2.position.z + bounds2.size.z/2}
    };
    return checkCapsuleCollision(capsule1, box);
}

bool CollisionSystem::checkCapsuleCollision(const CollisionBounds& capsule, const BoundingBox& box) {
    // Basic AABB-capsule check (placeholder, implement properly if needed)
    return CheckCollisionBoxSphere(box, capsule.position, capsule.size.x);
}

// resolveCollisions implementation
void CollisionSystem::resolveCollisions(Vector3& newPosition, const Vector3& originalPosition, float playerRadius, float playerHeight, float playerY, float eyeHeight, float groundLevel, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding) {
    // Create player capsule bounds
    CollisionBounds playerBounds;
    playerBounds.shape = CollisionShape::CAPSULE;
    playerBounds.position = {newPosition.x, playerY + playerHeight / 2, newPosition.z};  // Center of capsule
    playerBounds.size = {playerRadius, playerHeight, 0.0f};  // radius, height

    // Debug: Log collision check
    static int collisionDebugCounter = 0;
    if (collisionDebugCounter++ % 60 == 0) {
        std::cout << "COLLISION: Checking player at (" << newPosition.x << ", " << newPosition.y << ", " << newPosition.z
                  << ") bounds: (" << playerBounds.position.x << ", " << playerBounds.position.y << ", " << playerBounds.position.z << ")" << std::endl;
    }

    // Check collision with environment
    if (environment.checkCollision(playerBounds, isInBuilding ? currentBuilding : -1)) {
        // Debug: Collision detected
        std::cout << "COLLISION DETECTED: Reverting position from (" << newPosition.x << ", " << newPosition.y << ", " << newPosition.z
                  << ") to (" << originalPosition.x << ", " << originalPosition.y << ", " << originalPosition.z << ")" << std::endl;
        // Simple resolution: revert to original position
        newPosition = originalPosition;
        std::cout << "Collision resolved: reverted to original position" << std::endl;
    } else {
        // No collision, allow movement
    }

    // Ground check
    if (newPosition.y < groundLevel + eyeHeight) {
        newPosition.y = groundLevel + eyeHeight;
    }
}

// Door collision
bool CollisionSystem::checkDoorCollision(const CollisionBounds& playerBounds, const EnvironmentManager& environment, int& doorBuildingId) {
    auto objects = environment.getAllObjects();
    for (const auto& obj : objects) {
        if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
            // Check distance to door
            if (MathUtils::distance3D(playerBounds.position, building->getDoorPosition()) < DOOR_INTERACTION_DISTANCE) {
                doorBuildingId = building->getId();
                return true;
            }
        }
    }
    return false;
}

bool CollisionSystem::canEnterBuilding(int buildingId, const EnvironmentManager& environment) {
    // Placeholder: always allow
    return true;
}