// environmental_object.h (updated)
#ifndef ENVIRONMENTAL_OBJECT_H
#define ENVIRONMENTAL_OBJECT_H

#include "raylib.h"
#include "collision_system.h"  // For CollisionBounds
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>

class Component {
public:
    virtual ~Component() = default;
    virtual void update(float deltaTime) {}
    virtual std::string getTypeName() const = 0;
};

class RenderComponent : public Component {
public:
    virtual void render(const Camera3D& camera) = 0;
    std::string getTypeName() const override { return "RenderComponent"; }
};

class PhysicsComponent : public Component {
public:
    virtual CollisionBounds getBounds() const = 0;
    std::string getTypeName() const override { return "PhysicsComponent"; }
};

class EnvironmentalObject {
public:
    virtual ~EnvironmentalObject() = default;
    void addComponent(std::unique_ptr<Component> component);
    Component* getComponent(const std::string& typeName) const;
    void update(float deltaTime);
    void render(const Camera3D& camera);

    Vector3 position;
    bool collidable = true;
    virtual bool isInteractive() const { return false; }
    virtual void interact() {}
    virtual std::string getName() const { return "Object"; }
    virtual float getInteractionRadius() const { return 0.0f; }
    virtual CollisionBounds getCollisionBounds() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Component>> components_;
};

// Object pooling
template<typename T>
class ObjectPool {
public:
    std::unique_ptr<T> acquire();
    void release(std::unique_ptr<T> object);

private:
    std::vector<std::unique_ptr<T>> pool_;
    std::mutex poolMutex_;
};

// Factory configs (based on current constructors)
struct DoorConfig {
    Vector3 offset;
    float width = 1.2f;
    float height = 2.8f;
    float rotation = 0.0f;
    Color color = BROWN;
};

struct BuildingConfig {
    int id = -1;  // Unique building identifier
    Vector3 size;
    Color color;
    std::string name;
    DoorConfig door;
    bool canEnter = true;
};

struct WellConfig {
    float baseRadius = 1.5f;
    float height = 2.5f;
};

struct TreeConfig {
    float trunkRadius = 0.5f;
    float trunkHeight = 4.0f;
    float foliageRadius = 2.5f;
};

class EnvironmentalObjectFactory {
public:
    static std::shared_ptr<EnvironmentalObject> createBuilding(const BuildingConfig& config, Vector3 pos);
    static std::shared_ptr<EnvironmentalObject> createWell(const WellConfig& config, Vector3 pos);
    static std::shared_ptr<EnvironmentalObject> createTree(const TreeConfig& config, Vector3 pos);

    // Template-based generic creation
    template<typename T, typename... Args>
    static std::shared_ptr<T> create(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
};

// Derived classes refactored to use components

// Building-specific RenderComponent
class BuildingRenderComponent : public RenderComponent {
public:
    BuildingRenderComponent(const BuildingConfig& config);
    void render(const Camera3D& camera) override;

private:
    BuildingConfig config_;
};

// Building-specific PhysicsComponent
class BuildingPhysicsComponent : public PhysicsComponent {
public:
    BuildingPhysicsComponent(const Vector3& size);
    CollisionBounds getBounds() const override;

private:
    Vector3 size_;
};

class Building : public EnvironmentalObject {
public:
    Building(Vector3 pos, const BuildingConfig& config);
    virtual ~Building() = default;

    bool isInteractive() const override { return config_.canEnter; }
    std::string getName() const override { return config_.name; }
    float getInteractionRadius() const override { return 3.0f; }

    int getId() const { return config_.id; }
    Vector3 getDoorPosition() const;
    Vector3 getSize() const { return config_.size; }
    Color getColor() const { return config_.color; }
    bool isPlayerAtDoor(Vector3 playerPos, float threshold = 3.0f) const;

    // Door-specific collision bounds
    CollisionBounds getDoorCollisionBounds() const;
    CollisionBounds getWallCollisionBounds() const;  // Building walls excluding door

private:
    BuildingConfig config_;
};

// Similar for Well
class WellRenderComponent : public RenderComponent {
public:
    WellRenderComponent(const WellConfig& config);
    void render(const Camera3D& camera) override;

private:
    WellConfig config_;
};

class WellPhysicsComponent : public PhysicsComponent {
public:
    WellPhysicsComponent(float baseRadius, float height);
    CollisionBounds getBounds() const override;

private:
    float baseRadius_;
    float height_;
};

class Well : public EnvironmentalObject {
public:
    Well(Vector3 pos, const WellConfig& config);
    virtual ~Well() = default;

    std::string getName() const override { return "Town Well"; }

private:
    WellConfig config_;
};

// For Tree
class TreeRenderComponent : public RenderComponent {
public:
    TreeRenderComponent(const TreeConfig& config);
    void render(const Camera3D& camera) override;

private:
    TreeConfig config_;
};

class TreePhysicsComponent : public PhysicsComponent {
public:
    TreePhysicsComponent(float trunkRadius, float trunkHeight);
    CollisionBounds getBounds() const override;

private:
    float trunkRadius_;
    float trunkHeight_;
};

class Tree : public EnvironmentalObject {
public:
    Tree(Vector3 pos, const TreeConfig& config);
    virtual ~Tree() = default;

    std::string getName() const override { return "Tree"; }

private:
    TreeConfig config_;
};

#endif