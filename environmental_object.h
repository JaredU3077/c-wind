// environmental_object.h
#ifndef ENVIRONMENTAL_OBJECT_H
#define ENVIRONMENTAL_OBJECT_H

#include "raylib.h"
#include <string>
#include <memory>

// Forward declaration to avoid circular dependency
struct CollisionBounds;

class EnvironmentalObject {
public:
    virtual ~EnvironmentalObject() = default;

    virtual void render() = 0;
    virtual CollisionBounds getCollisionBounds() const = 0;
    virtual bool isInteractive() const { return false; }
    virtual void interact() {}
    virtual std::string getName() const { return "Object"; }
    virtual float getInteractionRadius() const { return 0.0f; }

    Vector3 position;
    bool collidable = true;
};

class Building : public EnvironmentalObject {
public:
    struct DoorConfig {
        Vector3 offset;        // Offset from building center
        float width = 1.2f;
        float height = 2.8f;
        float rotation = 0.0f; // Rotation around Y axis
        Color color = BROWN;
    };

    Building(Vector3 pos, Vector3 size, Color color, const std::string& name,
             const DoorConfig& door, bool canEnter = true);

    void render() override;
    CollisionBounds getCollisionBounds() const override;
    bool isInteractive() const override { return canEnter; }
    void interact() override {}

    std::string getName() const override { return name; }
    float getInteractionRadius() const override { return 3.0f; }

    Vector3 getDoorPosition() const;
    Vector3 getSize() const { return size; }
    Color getColor() const { return color; }
    bool isPlayerAtDoor(Vector3 playerPos, float threshold = 3.0f) const;

private:
    Vector3 size;
    Color color;
    std::string name;
    DoorConfig doorConfig;
    bool canEnter;
};

class Well : public EnvironmentalObject {
public:
    Well(Vector3 pos, float baseRadius = 1.5f, float height = 2.5f);

    void render() override;
    CollisionBounds getCollisionBounds() const override;
    std::string getName() const override { return "Town Well"; }

private:
    float baseRadius;
    float height;
};

class Tree : public EnvironmentalObject {
public:
    Tree(Vector3 pos, float trunkRadius = 0.5f, float trunkHeight = 4.0f, float foliageRadius = 2.5f);

    void render() override;
    CollisionBounds getCollisionBounds() const override;
    std::string getName() const override { return "Tree"; }

private:
    float trunkRadius;
    float trunkHeight;
    float foliageRadius;
};

#endif