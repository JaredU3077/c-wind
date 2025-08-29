// collision_system.h
#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "raylib.h"
#include "environment_manager.h"
#include "npc.h"  // For checkNPCCollision
#include <algorithm>  // For std::clamp
#include <cmath>

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
    static bool checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2);
    static bool checkPointInBounds(Vector3 point, const CollisionBounds& bounds);

    static void resolveCollisions(Vector3& newPosition, const Vector3& originalPosition, float playerRadius, float playerHeight, float playerY, float eyeHeight, float groundLevel, const EnvironmentManager& environment, bool isInBuilding, int currentBuilding);

private:
    static bool checkBoxCollision(const CollisionBounds& box1, const CollisionBounds& bounds2);
    static bool checkSphereCollision(const CollisionBounds& sphere1, const CollisionBounds& bounds2);
    static bool checkCylinderCollision(const CollisionBounds& cyl1, const CollisionBounds& bounds2);
    static bool checkCapsuleCollision(const CollisionBounds& capsule1, const CollisionBounds& bounds2);
    static bool pointInBox(Vector3 point, const CollisionBounds& box);
    static bool pointInSphere(Vector3 point, const CollisionBounds& sphere);
    static bool pointInCylinder(Vector3 point, const CollisionBounds& cylinder);
    static bool pointInCapsule(Vector3 point, const CollisionBounds& capsule);
};

#endif