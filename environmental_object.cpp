// environmental_object.cpp (updated)
#include "environmental_object.h"
#include "math_utils.h"
#include "collision_system.h"  // For CollisionBounds and CollisionShape
#include "rlgl.h"             // For rlPushMatrix, rlTranslatef, rlRotatef, rlPopMatrix
#include <cmath>
#include <iostream>

void EnvironmentalObject::addComponent(std::unique_ptr<Component> component) {
    components_[component->getTypeName()] = std::move(component);
}

Component* EnvironmentalObject::getComponent(const std::string& typeName) const {
    auto it = components_.find(typeName);
    return (it != components_.end()) ? it->second.get() : nullptr;
}

void EnvironmentalObject::update(float deltaTime) {
    for (auto& pair : components_) {
        pair.second->update(deltaTime);
    }
}

void EnvironmentalObject::render([[maybe_unused]] const Camera3D& camera) {
    auto* component = getComponent("RenderComponent");
    if (component) {
        auto renderComp = dynamic_cast<RenderComponent*>(component);
        if (renderComp) {
            // Apply position transformation before rendering
            rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);
            renderComp->render(camera);
            rlPopMatrix();
        } else {
            std::cerr << "ERROR: Component '" << component->getTypeName() << "' is not a RenderComponent for object '" << getName() << "'" << std::endl;
        }
    } else {
        std::cerr << "WARNING: No RenderComponent found for object '" << getName() << "'" << std::endl;
    }
}

CollisionBounds EnvironmentalObject::getCollisionBounds() const {
    if (auto physicsComp = dynamic_cast<PhysicsComponent*>(getComponent("PhysicsComponent"))) {
        CollisionBounds bounds = physicsComp->getBounds();
        // Convert relative bounds to world coordinates
        bounds.position.x += position.x;
        bounds.position.y += position.y;
        bounds.position.z += position.z;
        return bounds;
    }
    return CollisionBounds{}; // Return default bounds if no physics component
}

template<typename T>
std::unique_ptr<T> ObjectPool<T>::acquire() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    if (!pool_.empty()) {
        std::unique_ptr<T> obj = std::move(pool_.back());
        pool_.pop_back();
        return obj;
    }
    return std::make_unique<T>();
}

template<typename T>
void ObjectPool<T>::release(std::unique_ptr<T> object) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    pool_.push_back(std::move(object));
}

// Explicit instantiations if needed (e.g., for Building, Well, Tree)
// template class ObjectPool<Building>;
// template class ObjectPool<Well>;
// template class ObjectPool<Tree>;

// Factory implementations
std::shared_ptr<EnvironmentalObject> EnvironmentalObjectFactory::createBuilding(const BuildingConfig& config, Vector3 pos) {
    return std::make_shared<Building>(pos, config);
}

std::shared_ptr<EnvironmentalObject> EnvironmentalObjectFactory::createWell(const WellConfig& config, Vector3 pos) {
    return std::make_shared<Well>(pos, config);
}

std::shared_ptr<EnvironmentalObject> EnvironmentalObjectFactory::createTree(const TreeConfig& config, Vector3 pos) {
    return std::make_shared<Tree>(pos, config);
}

// Building Components
BuildingRenderComponent::BuildingRenderComponent(const BuildingConfig& config) : config_(config) {}

void BuildingRenderComponent::render([[maybe_unused]] const Camera3D& camera) {
    // Draw main building (position is from owner)
    DrawCube({0,0,0}, config_.size.x, config_.size.y, config_.size.z, config_.color);  // Relative to position
    DrawCubeWires({0,0,0}, config_.size.x, config_.size.y, config_.size.z, BLACK);

    // Roof
    Vector3 roofPos = {0, config_.size.y/2 + 0.5f, 0};
    DrawCube(roofPos, config_.size.x + 1.0f, 1.0f, config_.size.z + 1.0f, DARKGRAY);

    // Door
    Vector3 doorPos = config_.door.offset;
    rlPushMatrix();
    rlTranslatef(doorPos.x, doorPos.y + config_.door.height / 2.0f, doorPos.z);
    rlRotatef(config_.door.rotation, 0.0f, 1.0f, 0.0f);
    DrawCube({0, 0, 0}, config_.door.width, config_.door.height, 0.2f, config_.door.color);
    DrawCubeWires({0, 0, 0}, config_.door.width, config_.door.height, 0.2f, BLACK);
    rlPopMatrix();

    // Handle
    float handleX = cosf(config_.door.rotation * DEG2RAD) * 0.4f;
    float handleZ = sinf(config_.door.rotation * DEG2RAD) * 0.4f;
    Vector3 handlePos = {doorPos.x + handleX, doorPos.y + 1.0f, doorPos.z + handleZ};
    DrawSphere(handlePos, 0.08f, GOLD);

    // Sign
    Vector3 signPos = {doorPos.x, doorPos.y + 2.2f, doorPos.z};
    rlPushMatrix();
    rlTranslatef(signPos.x, signPos.y, signPos.z);
    rlRotatef(config_.door.rotation, 0.0f, 1.0f, 0.0f);
    DrawCube({0, 0, 0}, 1.5f, 0.3f, 0.05f, LIGHTGRAY);
    rlPopMatrix();
}

BuildingPhysicsComponent::BuildingPhysicsComponent(const Vector3& size) : size_(size) {}

CollisionBounds BuildingPhysicsComponent::getBounds() const {
    return {
        CollisionShape::BOX,
        {0, 0, 0},  // Position will be transformed by EnvironmentalObject::getCollisionBounds()
        {size_.x, size_.y, size_.z},
        0.0f
    };
}

Building::Building(Vector3 pos, const BuildingConfig& config) : config_(config) {
    position = pos;
    collidable = true;  // Ensure collision is enabled for buildings
    addComponent(std::make_unique<BuildingRenderComponent>(config));
    addComponent(std::make_unique<BuildingPhysicsComponent>(config.size));
}

Vector3 Building::getDoorPosition() const {
    return Vector3{position.x + config_.door.offset.x, position.y + config_.door.offset.y, position.z + config_.door.offset.z};
}

bool Building::isPlayerAtDoor(Vector3 playerPos, float threshold) const {
    Vector3 doorPos = getDoorPosition();
    float distance = MathUtils::distance3D(playerPos, doorPos);
    return distance <= threshold;
}

CollisionBounds Building::getDoorCollisionBounds() const {
    Vector3 doorPos = getDoorPosition();
    // Removed excessive debug logging
    return {
        CollisionShape::BOX,
        doorPos,
        {config_.door.width, config_.door.height, 0.2f},  // Slightly thicker door for better collision
        config_.door.rotation
    };
}

CollisionBounds Building::getWallCollisionBounds() const {
    // Return the building bounds excluding the door area
    return {
        CollisionShape::BOX,
        position,
        config_.size,
        0.0f
    };
}

// Well Components
WellRenderComponent::WellRenderComponent(const WellConfig& config) : config_(config) {}

void WellRenderComponent::render([[maybe_unused]] const Camera3D& camera) {
    DrawCylinder({0,0,0}, config_.baseRadius, config_.baseRadius, 0.5f, 16, DARKGRAY);
    DrawCylinder({0,0,0}, config_.baseRadius * 0.8f, config_.baseRadius * 0.8f, config_.height, 16, GRAY);
    DrawCylinder({0, config_.height, 0}, config_.baseRadius * 0.9f, config_.baseRadius * 0.7f, 0.2f, 16, DARKGRAY);
    DrawCylinder({0, config_.height + 0.5f, 0}, 0.1f, 0.1f, 1.0f, 8, BROWN);
    DrawSphere({0, config_.height + 1.5f, 0}, 0.15f, GRAY);
}

WellPhysicsComponent::WellPhysicsComponent(float baseRadius, float height) : baseRadius_(baseRadius), height_(height) {}

CollisionBounds WellPhysicsComponent::getBounds() const {
    return {
        CollisionShape::CYLINDER,
        {0,0,0},
        {baseRadius_, height_, 0.0f},
        0.0f
    };
}

Well::Well(Vector3 pos, const WellConfig& config) : config_(config) {
    position = pos;
    collidable = true;  // Ensure collision is enabled for the well
    addComponent(std::make_unique<WellRenderComponent>(config));
    addComponent(std::make_unique<WellPhysicsComponent>(config.baseRadius, config.height));
}

// Tree Components
TreeRenderComponent::TreeRenderComponent(const TreeConfig& config) : config_(config) {}

void TreeRenderComponent::render([[maybe_unused]] const Camera3D& camera) {
    // Draw trunk - positioned from ground level up with realistic tree colors
    Vector3 trunkPos = {0, config_.trunkHeight/2, 0};
    DrawCylinder(trunkPos, config_.trunkRadius, config_.trunkRadius, config_.trunkHeight, 16, Color{139, 69, 19, 255}); // Saddle brown
    DrawCylinderWires(trunkPos, config_.trunkRadius, config_.trunkRadius, config_.trunkHeight, 16, Color{101, 67, 33, 255}); // Darker brown

    // Draw foliage - layered spheres for more realistic tree shape
    // Main foliage layer at top of trunk
    Vector3 mainFoliagePos = {0, config_.trunkHeight - 0.3f, 0};
    DrawSphere(mainFoliagePos, config_.foliageRadius, Color{34, 139, 34, 255}); // Forest green
    DrawSphereWires(mainFoliagePos, config_.foliageRadius, 12, 8, Color{0, 100, 0, 255}); // Dark green wireframe

    // Upper foliage layer for tree-like shape
    Vector3 upperFoliagePos = {0, config_.trunkHeight + 0.5f, 0};
    DrawSphere(upperFoliagePos, config_.foliageRadius * 0.8f, Color{50, 160, 50, 255}); // Lighter green
    DrawSphereWires(upperFoliagePos, config_.foliageRadius * 0.8f, 10, 6, Color{25, 120, 25, 255});

    // Add some smaller branches for realism
    float branchY = config_.trunkHeight * 0.7f;
    Vector3 branchPos1 = {config_.trunkRadius + 0.2f, branchY, 0};
    Vector3 branchPos2 = {-config_.trunkRadius - 0.2f, branchY, 0};
    Vector3 branchPos3 = {0, branchY, config_.trunkRadius + 0.2f};

    // Draw small branch stubs
    DrawCylinder(branchPos1, 0.1f, 0.1f, 0.8f, 8, Color{101, 67, 33, 255});
    DrawCylinder(branchPos2, 0.1f, 0.1f, 0.8f, 8, Color{101, 67, 33, 255});
    DrawCylinder(branchPos3, 0.1f, 0.1f, 0.8f, 8, Color{101, 67, 33, 255});
}

TreePhysicsComponent::TreePhysicsComponent(float trunkRadius, float trunkHeight) : trunkRadius_(trunkRadius), trunkHeight_(trunkHeight) {}

CollisionBounds TreePhysicsComponent::getBounds() const {
    return {
        CollisionShape::CYLINDER,
        {0,0,0},
        {trunkRadius_, trunkHeight_, 0.0f},
        0.0f
    };
}

Tree::Tree(Vector3 pos, const TreeConfig& config) : config_(config) {
    position = pos;
    collidable = true;  // Ensure collision is enabled for trees
    addComponent(std::make_unique<TreeRenderComponent>(config));
    addComponent(std::make_unique<TreePhysicsComponent>(config.trunkRadius, config.trunkHeight));
}