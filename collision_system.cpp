// collision_system.cpp
#include "collision_system.h"
#include "environmental_object.h"  // For Building
#include "environment_manager.h"   // For EnvironmentManager

bool CollisionSystem::checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2) {
    // Removed excessive debug logging for performance
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

BoundingBox CollisionSystem::boundsToBox(const CollisionBounds& bounds) {
    BoundingBox box;
    float halfWidth = bounds.size.x / 2.0f;
    float halfHeight = bounds.size.y / 2.0f;
    float halfDepth = bounds.size.z / 2.0f;

    box.min.x = bounds.position.x - halfWidth;
    box.min.y = bounds.position.y - halfHeight;
    box.min.z = bounds.position.z - halfDepth;

    box.max.x = bounds.position.x + halfWidth;
    box.max.y = bounds.position.y + halfHeight;
    box.max.z = bounds.position.z + halfDepth;

    return box;
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

            bool collision = (maxX1 >= minX2 && minX1 <= maxX2) &&
                           (maxY1 >= minY2 && minY1 <= maxY2) &&
                           (maxZ1 >= minZ2 && minZ1 <= maxZ2);



            return collision;
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

            bool withinRadius = dist2D <= cyl1.size.x;
            if (!withinRadius) return false;

            float cylBottom = cyl1.position.y;
            float cylTop = cyl1.position.y + cyl1.size.y;
            bool withinHeight = (cylTop > minY && cylBottom < maxY);
            bool collision = withinRadius && withinHeight;



            return collision;
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
    // **CRITICAL FIX**: Never exclude buildings from collision detection
    // The building detection logic has race conditions that cause wall penetration
    // Buildings should ALWAYS block movement until we fix the detection logic
    int excludeIndex = -1;  // Never exclude any objects from collision

    // **SIMPLIFIED FIX**: Use the camera position directly, adjust Y to ground level
    // The newPosition.y is camera height (eye level), so player feet are at newPosition.y - eyeHeight
    Vector3 boundsPos = newPosition;
    boundsPos.y = newPosition.y - eyeHeight + (playerHeight / 2.0f);  // Center capsule on player body
    
    CollisionBounds playerBounds = {
        CollisionShape::CAPSULE,
        boundsPos,
        {playerRadius, playerHeight, playerRadius},  // CRITICAL FIX: Capsule needs radius in Z too!
        0.0f
    };

    // Debug collision bounds occasionally 
    static int boundsDebugCounter = 0;
    boundsDebugCounter++;
    if (boundsDebugCounter % 60 == 0) {
        printf("COLLISION BOUNDS: Player at (%.1f, %.1f, %.1f), radius: %.1f, height: %.1f\n",
               boundsPos.x, boundsPos.y, boundsPos.z, playerRadius, playerHeight);
    }

    // **SIMPLIFIED**: Disable door collision exception for now
    // The door/building detection logic has race conditions
    // Focus on making walls solid first, then re-enable door entry later
    
    // TODO: Re-implement door collision logic after fixing building detection race conditions

    // **SIMPLIFIED**: Always check collision against all objects
    bool wallCollision = environment.checkCollision(playerBounds, -1);  // Never exclude anything

    if (wallCollision) {
        // **IMPROVED**: Use more granular movement steps to prevent clipping through thin walls
        Vector3 movement = {
            newPosition.x - originalPosition.x,
            newPosition.y - originalPosition.y,
            newPosition.z - originalPosition.z
        };

        // **SIMPLIFIED**: Use fixed number of steps to avoid over-complication
        const int maxSteps = 10;  // Keep it simple and reliable
        
        Vector3 testPosition = originalPosition;
        bool foundValidPosition = false;

        for (int step = 1; step <= maxSteps && !foundValidPosition; ++step) {
            float t = (float)step / maxSteps;
            testPosition.x = originalPosition.x + movement.x * t;
            testPosition.y = originalPosition.y + movement.y * t;
            testPosition.z = originalPosition.z + movement.z * t;

            Vector3 testBoundsPos = testPosition;
            testBoundsPos.y = testPosition.y - eyeHeight + (playerHeight / 2.0f);  // Fix Y coordinate
            CollisionBounds testBounds = {
                CollisionShape::CAPSULE,
                testBoundsPos,
                {playerRadius, playerHeight, playerRadius},  // Fix Z dimension
                0.0f
            };

            if (!environment.checkCollision(testBounds, excludeIndex)) {
                newPosition = testPosition;
                foundValidPosition = true;
            }
        }

        // **IMPROVED**: Enhanced wall sliding with better collision bounds
        if (!foundValidPosition) {
            // Try sliding along X axis (keep original Z and Y)
            Vector3 slidePosition = originalPosition;
            slidePosition.x = newPosition.x;
            slidePosition.y = newPosition.y;  // Allow Y movement (jumping/falling)

            Vector3 slideBoundsPos = slidePosition;
            slideBoundsPos.y = slidePosition.y - eyeHeight + (playerHeight / 2.0f);  // Fix Y coordinate
            CollisionBounds slideBounds = {
                CollisionShape::CAPSULE,
                slideBoundsPos,
                {playerRadius, playerHeight, playerRadius},  // Fix Z dimension
                0.0f
            };

            if (!environment.checkCollision(slideBounds, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }

            // Try sliding along Z axis (keep original X)
            slidePosition = originalPosition;
            slidePosition.z = newPosition.z;
            slidePosition.y = newPosition.y;  // Allow Y movement

            slideBoundsPos = slidePosition;
            slideBoundsPos.y = slidePosition.y - eyeHeight + (playerHeight / 2.0f);  // Fix Y coordinate
            slideBounds = {
                CollisionShape::CAPSULE,
                slideBoundsPos,
                {playerRadius, playerHeight, playerRadius},  // Fix Z dimension
                0.0f
            };

            if (!environment.checkCollision(slideBounds, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }

            // Try Y-only movement (for jumping/falling against walls)
            slidePosition = originalPosition;
            slidePosition.y = newPosition.y;

            slideBoundsPos = slidePosition;
            slideBoundsPos.y = slidePosition.y - eyeHeight + (playerHeight / 2.0f);
            slideBounds = {
                CollisionShape::CAPSULE,
                slideBoundsPos,
                {playerRadius, playerHeight, playerRadius},  // Fix Z dimension
                0.0f
            };

            if (!environment.checkCollision(slideBounds, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }

            // If all else fails, don't move at all
            newPosition = originalPosition;
            printf("COLLISION: All movement blocked, staying at original position\n");
        }
    }

    // Check NPC collisions (always block movement) with corrected Y coordinate
    Vector3 npcPos = newPosition;
    npcPos.y = newPosition.y - eyeHeight + (playerHeight / 2.0f);  // Use consistent Y calculation
    if (checkNPCCollision(npcPos, playerRadius, playerHeight)) {
        newPosition = originalPosition;
    }

    if (isInBuilding) {
        auto objects = environment.getAllObjects();
        // Find building by ID instead of array index
        Building* currentBuildingPtr = nullptr;
        for (const auto& obj : objects) {
            if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                if (building->getId() == currentBuilding) {
                    currentBuildingPtr = building.get();
                    break;
                }
            }
        }

        if (currentBuildingPtr) {
                Vector3 bSize = currentBuildingPtr->getSize();
                Vector3 bPos = currentBuildingPtr->position;

                // More generous bounds for interior movement
                float halfW = bSize.x / 2.0f - playerRadius * 1.5f; // More space
                float halfH = bSize.y / 2.0f - 0.5f; // More headroom
                float halfD = bSize.z / 2.0f - playerRadius * 1.5f; // More space

                float minX = bPos.x - halfW;
                float maxX = bPos.x + halfW;
                float minY = groundLevel;
                float maxY = bPos.y + halfH;
                float minZ = bPos.z - halfD;
                float maxZ = bPos.z + halfD;

                // Clamp position but allow more freedom
                newPosition.x = std::max(minX, std::min(maxX, newPosition.x));
                newPosition.y = std::max(minY + eyeHeight, std::min(maxY, newPosition.y));
                newPosition.z = std::max(minZ, std::min(maxZ, newPosition.z));

                // Debug logging for building movement
                static int frameCounter = 0;
                frameCounter++;
                if (frameCounter % 60 == 0) {
                    printf("Building bounds: X[%.1f, %.1f] Z[%.1f, %.1f] | Player: (%.1f, %.1f, %.1f)\n",
                           minX, maxX, minZ, maxZ, newPosition.x, newPosition.y, newPosition.z);
                }
            }
        }
    }

// Door-specific collision checking implementation
bool CollisionSystem::checkDoorCollision(const CollisionBounds& playerBounds, const EnvironmentManager& environment, int& doorBuildingId) {
    auto objects = environment.getAllObjects();



    for (size_t i = 0; i < objects.size(); ++i) {
        if (auto building = std::dynamic_pointer_cast<Building>(objects[i])) {
            if (building->isInteractive()) {
                CollisionBounds doorBounds = building->getDoorCollisionBounds();

                if (checkCollision(playerBounds, doorBounds)) {
                    doorBuildingId = building->getId();
                    return true;
                }
            }
        }
    }

    return false;
}

bool CollisionSystem::canEnterBuilding(int buildingId, const EnvironmentManager& environment) {
    auto objects = environment.getAllObjects();

    for (const auto& obj : objects) {
        if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
            if (building->getId() == buildingId && building->isInteractive()) {
                return true;
            }
        }
    }
    return false;
}