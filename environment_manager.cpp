// environment_manager.cpp
#include "environment_manager.h"

void EnvironmentManager::addObject(std::shared_ptr<EnvironmentalObject> obj) {
    objects.push_back(obj);
}

void EnvironmentManager::renderAll() {
    for (auto& obj : objects) {
        obj->render();
    }
}

bool EnvironmentManager::checkCollision(const CollisionBounds& bounds, int excludeIndex) const {
    for (size_t i = 0; i < objects.size(); ++i) {
        if (static_cast<int>(i) == excludeIndex) continue;
        if (objects[i]->collidable) {
            auto objBounds = objects[i]->getCollisionBounds();
            if (CollisionSystem::checkCollision(bounds, objBounds)) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::shared_ptr<EnvironmentalObject>> EnvironmentManager::getInteractiveObjects() const {
    std::vector<std::shared_ptr<EnvironmentalObject>> interactive;
    for (const auto& obj : objects) {
        if (obj->isInteractive()) {
            interactive.push_back(obj);
        }
    }
    return interactive;
}

const std::vector<std::shared_ptr<EnvironmentalObject>>& EnvironmentManager::getAllObjects() const {
    return objects;
}