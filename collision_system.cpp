#include "collision_system.h"
#include "environment_manager.h"  // Now included for full use
#include "math_utils.h"  // For MathUtils::distance3D

// Implementation for checkPointInBounds (focus on BOX for buildings)
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
    // Stub - implement if needed
    return false;
}

bool CollisionSystem::pointInCapsule(Vector3 point, const CollisionBounds& capsule) {
    // Stub - implement if needed
    return false;
}

// Stubs for other methods (unchanged)
BoundingBox CollisionSystem::boundsToBox(const CollisionBounds& bounds) {
    Vector3 min = {bounds.position.x - bounds.size.x / 2, bounds.position.y - bounds.size.y / 2, bounds.position.z - bounds.size.z / 2};
    Vector3 max = {bounds.position.x + bounds.size.x / 2, bounds.position.y + bounds.size.y / 2, bounds.position.z + bounds.size.z / 2};
    return {min, max};
}

bool CollisionSystem::checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2) {
    // Basic box-box for now
    BoundingBox box1 = boundsToBox(bounds1);
    BoundingBox box2 = boundsToBox(bounds2);
    return CheckCollisionBoxes(box1, box2);
}

void CollisionSystem::resolveCollisions(Vector3& newPosition, const Vector3& originalPosition, float playerRadius, float playerHeight, float playerY, float eyeHeight, float groundLevel, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding) {
    // Stub - expand as needed
}

bool CollisionSystem::checkDoorCollision(const CollisionBounds& playerBounds, const EnvironmentManager& environment, int& doorBuildingId) {
    // Stub - expand if needed
    return false;
}

bool CollisionSystem::canEnterBuilding(int buildingId, const EnvironmentManager& environment) {
    // Stub - expand if needed
    return true;
}

// Private stubs (implement as needed)
bool CollisionSystem::checkBoxCollision(const CollisionBounds& box1, const CollisionBounds& bounds2) {
    return false;
}

bool CollisionSystem::checkSphereCollision(const CollisionBounds& sphere1, const CollisionBounds& bounds2) {
    return false;
}

bool CollisionSystem::checkCylinderCollision(const CollisionBounds& cyl1, const CollisionBounds& bounds2) {
    return false;
}

bool CollisionSystem::checkCapsuleCollision(const CollisionBounds& capsule1, const CollisionBounds& bounds2) {
    return false;
}