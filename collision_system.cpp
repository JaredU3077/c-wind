// collision_system.cpp
#include "collision_system.h"
#include "environmental_object.h"  // For Building

bool CollisionSystem::checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2) {
    switch (bounds1.shape) {
        case CollisionShape::BOX:
            return checkBoxCollision(bounds1, bounds2);
        case CollisionShape::SPHERE:
            return checkSphereCollision(bounds1, bounds2);
        case CollisionShape::CYLINDER:
            return checkCylinderCollision(bounds1, bounds2);
        case CollisionShape::CAPSULE:
            return checkCapsuleCollision(bounds1, bounds2);
        default:
            return false;
    }
}

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
        default:
            return false;
    }
}

bool CollisionSystem::checkBoxCollision(const CollisionBounds& box1, const CollisionBounds& bounds2) {
    float halfWidth1 = box1.size.x / 2.0f;
    float halfHeight1 = box1.size.y / 2.0f;
    float halfDepth1 = box1.size.z / 2.0f;

    float minX1 = box1.position.x - halfWidth1;
    float maxX1 = box1.position.x + halfWidth1;
    float minY1 = box1.position.y - halfHeight1;
    float maxY1 = box1.position.y + halfHeight1;
    float minZ1 = box1.position.z - halfDepth1;
    float maxZ1 = box1.position.z + halfDepth1;

    switch (bounds2.shape) {
        case CollisionShape::BOX: {
            float halfWidth2 = bounds2.size.x / 2.0f;
            float halfHeight2 = bounds2.size.y / 2.0f;
            float halfDepth2 = bounds2.size.z / 2.0f;

            float minX2 = bounds2.position.x - halfWidth2;
            float maxX2 = bounds2.position.x + halfWidth2;
            float minY2 = bounds2.position.y - halfHeight2;
            float maxY2 = bounds2.position.y + halfHeight2;
            float minZ2 = bounds2.position.z - halfDepth2;
            float maxZ2 = bounds2.position.z + halfDepth2;

            return (maxX1 >= minX2 && minX1 <= maxX2) &&
                   (maxY1 >= minY2 && minY1 <= maxY2) &&
                   (maxZ1 >= minZ2 && minZ1 <= maxZ2);
        }
        case CollisionShape::SPHERE: {
            float closestX = std::max(minX1, std::min(bounds2.position.x, maxX1));
            float closestY = std::max(minY1, std::min(bounds2.position.y, maxY1));
            float closestZ = std::max(minZ1, std::min(bounds2.position.z, maxZ1));

            float distanceX = bounds2.position.x - closestX;
            float distanceY = bounds2.position.y - closestY;
            float distanceZ = bounds2.position.z - closestZ;

            return (distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ) <= (bounds2.size.x * bounds2.size.x);
        }
        case CollisionShape::CYLINDER: {
            // Simplified box-cylinder collision
            return (bounds2.position.x >= minX1 && bounds2.position.x <= maxX1) &&
                   (bounds2.position.y >= minY1 && bounds2.position.y <= maxY1) &&
                   (bounds2.position.z >= minZ1 && bounds2.position.z <= maxZ1);
        }
        default:
            return false;
    }
}

bool CollisionSystem::checkSphereCollision(const CollisionBounds& sphere1, const CollisionBounds& bounds2) {
    float distance = sqrtf(
        (sphere1.position.x - bounds2.position.x) * (sphere1.position.x - bounds2.position.x) +
        (sphere1.position.y - bounds2.position.y) * (sphere1.position.y - bounds2.position.y) +
        (sphere1.position.z - bounds2.position.z) * (sphere1.position.z - bounds2.position.z)
    );

    switch (bounds2.shape) {
        case CollisionShape::SPHERE:
            return distance <= (sphere1.size.x + bounds2.size.x);
        case CollisionShape::CYLINDER:
            return distance <= (sphere1.size.x + bounds2.size.x);
        default:
            return false;
    }
}

bool CollisionSystem::checkCylinderCollision(const CollisionBounds& cyl1, const CollisionBounds& bounds2) {
    float distance = sqrtf(
        (cyl1.position.x - bounds2.position.x) * (cyl1.position.x - bounds2.position.x) +
        (cyl1.position.z - bounds2.position.z) * (cyl1.position.z - bounds2.position.z)
    );

    switch (bounds2.shape) {
        case CollisionShape::CYLINDER:
            return distance <= (cyl1.size.x + bounds2.size.x);
        case CollisionShape::SPHERE:
            return distance <= (cyl1.size.x + bounds2.size.x);
        case CollisionShape::BOX: {
            float halfWidth = bounds2.size.x / 2.0f;
            float halfHeight = bounds2.size.y / 2.0f;
            float halfDepth = bounds2.size.z / 2.0f;

            float minX = bounds2.position.x - halfWidth;
            float maxX = bounds2.position.x + halfWidth;
            float minY = bounds2.position.y - halfHeight;
            float maxY = bounds2.position.y + halfHeight;
            float minZ = bounds2.position.z - halfDepth;
            float maxZ = bounds2.position.z + halfDepth;

            float closestX = std::max(minX, std::min(cyl1.position.x, maxX));
            float closestZ = std::max(minZ, std::min(cyl1.position.z, maxZ));

            float dx = closestX - cyl1.position.x;
            float dz = closestZ - cyl1.position.z;
            float dist2D = sqrtf(dx * dx + dz * dz);

            if (dist2D > cyl1.size.x) return false;

            float cylBottom = cyl1.position.y;
            float cylTop = cyl1.position.y + cyl1.size.y;
            return (cylTop > minY && cylBottom < maxY);
        }
        default:
            return false;
    }
}

bool CollisionSystem::checkCapsuleCollision(const CollisionBounds& capsule1, const CollisionBounds& bounds2) {
    return checkCylinderCollision(capsule1, bounds2);
}

bool CollisionSystem::pointInBox(Vector3 point, const CollisionBounds& box) {
    float halfWidth = box.size.x / 2.0f;
    float halfHeight = box.size.y / 2.0f;
    float halfDepth = box.size.z / 2.0f;

    return (point.x >= box.position.x - halfWidth && point.x <= box.position.x + halfWidth) &&
           (point.y >= box.position.y - halfHeight && point.y <= box.position.y + halfHeight) &&
           (point.z >= box.position.z - halfDepth && point.z <= box.position.z + halfDepth);
}

bool CollisionSystem::pointInSphere(Vector3 point, const CollisionBounds& sphere) {
    float distance = sqrtf(
        (point.x - sphere.position.x) * (point.x - sphere.position.x) +
        (point.y - sphere.position.y) * (point.y - sphere.position.y) +
        (point.z - sphere.position.z) * (point.z - sphere.position.z)
    );
    return distance <= sphere.size.x;
}

bool CollisionSystem::pointInCylinder(Vector3 point, const CollisionBounds& cylinder) {
    if (point.y < cylinder.position.y || point.y > cylinder.position.y + cylinder.size.y) {
        return false;
    }

    float distance = sqrtf(
        (point.x - cylinder.position.x) * (point.x - cylinder.position.x) +
        (point.z - cylinder.position.z) * (point.z - cylinder.position.z)
    );
    return distance <= cylinder.size.x;
}

bool CollisionSystem::pointInCapsule(Vector3 point, const CollisionBounds& capsule) {
    return pointInCylinder(point, capsule);
}

void CollisionSystem::resolveCollisions(Vector3& newPosition, const Vector3& originalPosition, float playerRadius, float playerHeight, float playerY, float eyeHeight, float groundLevel, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding) {
    int excludeIndex = isInBuilding ? currentBuilding : -1;

    Vector3 boundsPos = newPosition;
    boundsPos.y = playerY;
    CollisionBounds playerBounds = {
        CollisionShape::CAPSULE,
        boundsPos,
        {playerRadius, playerHeight, 0.0f},
        0.0f
    };

    if (environment.checkCollision(playerBounds, excludeIndex)) {
        Vector3 slidePosition = newPosition;
        Vector3 movement = {
            newPosition.x - originalPosition.x,
            newPosition.y - originalPosition.y,
            newPosition.z - originalPosition.z
        };

        const float WALL_SLIDE_THRESHOLD = 0.1f;

        if (fabsf(movement.x) > WALL_SLIDE_THRESHOLD || fabsf(movement.z) > WALL_SLIDE_THRESHOLD) {
            slidePosition.x = newPosition.x;
            slidePosition.z = originalPosition.z + (movement.z * 0.7f);
            Vector3 slideBoundsPos = slidePosition;
            slideBoundsPos.y = playerY;
            CollisionBounds slideBounds = {
                CollisionShape::CAPSULE,
                slideBoundsPos,
                {playerRadius, playerHeight, 0.0f},
                0.0f
            };
            if (!environment.checkCollision(slideBounds, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }

            slidePosition = newPosition;
            slidePosition.z = newPosition.z;
            slidePosition.x = originalPosition.x + (movement.x * 0.7f);
            slideBoundsPos = slidePosition;
            slideBoundsPos.y = playerY;
            slideBounds = {
                CollisionShape::CAPSULE,
                slideBoundsPos,
                {playerRadius, playerHeight, 0.0f},
                0.0f
            };
            if (!environment.checkCollision(slideBounds, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }

            slidePosition = newPosition;
            slidePosition.x = originalPosition.x + (movement.x * 0.5f);
            slidePosition.z = originalPosition.z + (movement.z * 0.5f);
            slideBoundsPos = slidePosition;
            slideBoundsPos.y = playerY;
            slideBounds = {
                CollisionShape::CAPSULE,
                slideBoundsPos,
                {playerRadius, playerHeight, 0.0f},
                0.0f
            };
            if (!environment.checkCollision(slideBounds, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }
        }

        slidePosition = originalPosition;
        slidePosition.x += movement.x * 0.3f;
        slidePosition.z += movement.z * 0.3f;
        Vector3 slideBoundsPos = slidePosition;
        slideBoundsPos.y = playerY;
        CollisionBounds slideBounds = {
            CollisionShape::CAPSULE,
            slideBoundsPos,
            {playerRadius, playerHeight, 0.0f},
            0.0f
        };
        if (!environment.checkCollision(slideBounds, excludeIndex)) {
            newPosition = slidePosition;
            return;
        }

        newPosition = originalPosition;
    }

    Vector3 npcPos = newPosition;
    npcPos.y = playerY;
    if (checkNPCCollision(npcPos, playerRadius, playerHeight)) {
        newPosition = originalPosition;
    }

    if (isInBuilding) {
        auto objects = environment.getAllObjects();
        if (currentBuilding >= 0 && currentBuilding < static_cast<int>(objects.size())) {
            if (auto building = std::dynamic_pointer_cast<Building>(objects[currentBuilding])) {
                Vector3 bSize = building->getSize();
                Vector3 bPos = building->position;

                float halfW = bSize.x / 2.0f - playerRadius;
                float halfH = bSize.y / 2.0f - 0.1f; // Slight headroom
                float halfD = bSize.z / 2.0f - playerRadius;

                float minX = bPos.x - halfW;
                float maxX = bPos.x + halfW;
                float minY = groundLevel;
                float maxY = bPos.y + halfH;
                float minZ = bPos.z - halfD;
                float maxZ = bPos.z + halfD;

                newPosition.x = std::clamp(newPosition.x, minX, maxX);
                newPosition.y = std::clamp(newPosition.y, minY + eyeHeight, maxY);
                newPosition.z = std::clamp(newPosition.z, minZ, maxZ);
            }
        }
    }
}