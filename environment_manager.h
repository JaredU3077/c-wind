// environment_manager.h
#ifndef ENVIRONMENT_MANAGER_H
#define ENVIRONMENT_MANAGER_H

#include "environmental_object.h"
#include "collision_system.h"
#include <vector>
#include <memory>

class EnvironmentManager {
public:
    void addObject(std::shared_ptr<EnvironmentalObject> obj);
    void renderAll();
    bool checkCollision(const CollisionBounds& bounds, int excludeIndex = -1) const;
    std::vector<std::shared_ptr<EnvironmentalObject>> getInteractiveObjects() const;
    const std::vector<std::shared_ptr<EnvironmentalObject>>& getAllObjects() const;

private:
    std::vector<std::shared_ptr<EnvironmentalObject>> objects;
};

#endif