// environmental_object.cpp
#include "environmental_object.h"
#include <cmath>

Building::Building(Vector3 pos, Vector3 size, Color color, const std::string& name,
                   const DoorConfig& door, bool canEnter)
    : size(size), color(color), name(name), doorConfig(door), canEnter(canEnter) {
    position = pos;
}

void Building::render() {
    // Draw main building
    DrawCube(position, size.x, size.y, size.z, color);
    DrawCubeWires(position, size.x, size.y, size.z, BLACK);

    // Draw roof
    Vector3 roofPos = {position.x, position.y + size.y/2 + 0.5f, position.z};
    DrawCube(roofPos, size.x + 1.0f, 1.0f, size.z + 1.0f, DARKGRAY);

    // Draw properly mounted door with rotation support
    Vector3 doorPos = {
        position.x + doorConfig.offset.x,
        position.y + doorConfig.offset.y,
        position.z + doorConfig.offset.z
    };

    rlPushMatrix();
    rlTranslatef(doorPos.x, doorPos.y + doorConfig.height / 2.0f, doorPos.z); // Center for rotation
    rlRotatef(doorConfig.rotation, 0.0f, 1.0f, 0.0f);
    DrawCube({0, 0, 0}, doorConfig.width, doorConfig.height, 0.2f, doorConfig.color);
    DrawCubeWires({0, 0, 0}, doorConfig.width, doorConfig.height, 0.2f, BLACK);
    rlPopMatrix();

    // Draw door handle (adjusted for rotation)
    float handleX = cosf(doorConfig.rotation * DEG2RAD) * 0.4f;
    float handleZ = sinf(doorConfig.rotation * DEG2RAD) * 0.4f;
    Vector3 handlePos = {
        doorPos.x + handleX,
        doorPos.y + 1.0f,
        doorPos.z + handleZ
    };
    DrawSphere(handlePos, 0.08f, GOLD);

    // Draw building sign (rotated if necessary)
    Vector3 signPos = {doorPos.x, doorPos.y + 2.2f, doorPos.z};
    rlPushMatrix();
    rlTranslatef(signPos.x, signPos.y, signPos.z);
    rlRotatef(doorConfig.rotation, 0.0f, 1.0f, 0.0f);
    DrawCube({0, 0, 0}, 1.5f, 0.3f, 0.05f, LIGHTGRAY);
    rlPopMatrix();
}

CollisionBounds Building::getCollisionBounds() const {
    return {
        CollisionShape::BOX,
        position,
        {size.x, size.y, size.z},
        0.0f
    };
}

Vector3 Building::getDoorPosition() const {
    return {
        position.x + doorConfig.offset.x,
        position.y + doorConfig.offset.y,
        position.z + doorConfig.offset.z
    };
}

bool Building::isPlayerAtDoor(Vector3 playerPos, float threshold) const {
    Vector3 doorPos = getDoorPosition();
    float distance = sqrtf(
        (playerPos.x - doorPos.x) * (playerPos.x - doorPos.x) +
        (playerPos.y - doorPos.y) * (playerPos.y - doorPos.y) +
        (playerPos.z - doorPos.z) * (playerPos.z - doorPos.z)
    );
    return distance <= threshold;
}

Well::Well(Vector3 pos, float baseRadius, float height)
    : baseRadius(baseRadius), height(height) {
    position = pos;
}

void Well::render() {
    Vector3 wellBase = position;
    Vector3 wellTop = {position.x, position.y + height, position.z};

    DrawCylinder(wellBase, baseRadius, baseRadius, 0.5f, 16, DARKGRAY);
    DrawCylinder(wellBase, baseRadius * 0.8f, baseRadius * 0.8f, height, 16, GRAY);
    DrawCylinder(wellTop, baseRadius * 0.9f, baseRadius * 0.7f, 0.2f, 16, DARKGRAY);
    DrawCylinder({position.x, position.y + height + 0.5f, position.z}, 0.1f, 0.1f, 1.0f, 8, BROWN);
    DrawSphere({position.x, position.y + height + 1.5f, position.z}, 0.15f, GRAY);
}

CollisionBounds Well::getCollisionBounds() const {
    return {
        CollisionShape::CYLINDER,
        position,
        {baseRadius, height, 0.0f},
        0.0f
    };
}

Tree::Tree(Vector3 pos, float trunkRadius, float trunkHeight, float foliageRadius)
    : trunkRadius(trunkRadius), trunkHeight(trunkHeight), foliageRadius(foliageRadius) {
    position = pos;
}

void Tree::render() {
    DrawCylinder(position, trunkRadius, trunkRadius, trunkHeight, 8, DARKBROWN);
    Vector3 foliagePos = {position.x, position.y + trunkHeight, position.z};
    DrawSphere(foliagePos, foliageRadius, GREEN);
}

CollisionBounds Tree::getCollisionBounds() const {
    return {
        CollisionShape::CYLINDER,
        position,
        {trunkRadius, trunkHeight, 0.0f},
        0.0f
    };
}