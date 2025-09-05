// environment_manager.cpp (updated)
#include "environment_manager.h"
#include "math_utils.h"
#include "raylib.h"
#include <cmath>
#include <iostream>  // For error logging

void EnvironmentManager::addObject(std::shared_ptr<EnvironmentalObject> obj) {
    objects.push_back(obj);

    // Initialize spatial grid if not already done
    if (spatialGrid_.isEmpty()) {
        spatialGrid_.rebuild();
    }

    // Insert object into spatial grid if collidable
    if (obj->collidable) {
        spatialGrid_.insert(obj);
    }

    printf("Added object: %s at (%.1f, %.1f, %.1f), collidable: %d\n",
           obj->getName().c_str(), obj->position.x, obj->position.y, obj->position.z, obj->collidable);
}

void EnvironmentManager::rebuildSpatialGrid() {
    printf("ENVIRONMENT: Rebuilding spatial grid with %zu objects\n", objects.size());
    spatialGrid_.rebuildWithObjects(objects);
}

void EnvironmentManager::update(float deltaTime, const Camera3D& camera) {
    for (auto& obj : objects) {
        obj->update(deltaTime);
    }
    lodManager_.updateLODLevels(camera, objects);
    asyncLoader_.processCompletedLoads(this);


}

void EnvironmentManager::renderAll(const Camera3D& camera) {
    for (auto& obj : objects) {
        obj->render(camera);
    }
}

bool EnvironmentManager::checkCollision(const CollisionBounds& bounds, int excludeIndex) const {
    // **SIMPLIFIED APPROACH**: Skip broken spatial grid entirely and use reliable brute force
    // This is more predictable and easier to debug than a complex spatial partitioning system
    auto candidates = objects;  // Always check all objects - it's only 5 objects, very fast
    
    // Debug: Log collision check very rarely (only when debugging)
    #ifdef BROWSERWIND_DEBUG
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 1800 == 0) {  // Once every 30 seconds at 60 FPS
        BoundingBox queryBox = CollisionSystem::boundsToBox(bounds);
        printf("COLLISION CHECK: Player bounds (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f), exclude: %d\n",
               queryBox.min.x, queryBox.min.y, queryBox.min.z,
               queryBox.max.x, queryBox.max.y, queryBox.max.z, excludeIndex);
    }
    #endif

    for (const auto& obj : candidates) {
        if (obj->collidable) {
            // Check if this object should be excluded
            bool shouldExclude = false;

            // If excludeIndex is -1, exclude nothing
            if (excludeIndex != -1) {
                // Try to cast to Building and check ID
                if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                    if (building->getId() == excludeIndex) {
                        shouldExclude = true;
                    }
                }
                // For non-buildings, fall back to array index exclusion (for backward compatibility)
                else {
                    // Find the array index of this object
                    auto it = std::find(objects.begin(), objects.end(), obj);
                    if (it != objects.end()) {
                        size_t arrayIndex = std::distance(objects.begin(), it);
                        if (static_cast<int>(arrayIndex) == excludeIndex) {
                            shouldExclude = true;
                        }
                    }
                }
            }

            if (!shouldExclude) {
                auto objBounds = obj->getCollisionBounds();
                if (CollisionSystem::checkCollision(bounds, objBounds)) {
                    #ifdef BROWSERWIND_DEBUG
                    if (debugCounter % 1800 == 0) {  // Very rare debug frequency
                        printf("  COLLISION DETECTED: Player hit %s at (%.1f,%.1f,%.1f)!\n",
                               obj->getName().c_str(),
                               objBounds.position.x, objBounds.position.y, objBounds.position.z);
                    }
                    #endif
                    return true;
                }
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

void EnvironmentManager::loadObjectAsync(const std::string& configPath, LoadCallback callback) {
    asyncLoader_.loadObjectAsync(configPath, callback);
}

// SpatialGrid implementations
size_t EnvironmentManager::SpatialGrid::getFlatIndex(int x, int y, int z) const {
    return (x * GRID_SIZE * GRID_SIZE) + (y * GRID_SIZE) + z;
}

void EnvironmentManager::SpatialGrid::insert(std::shared_ptr<EnvironmentalObject> obj) {
    // Calculate 3D indices first
    int x = static_cast<int>((obj->position.x - worldMin_.x) / cellSize_);
    int y = static_cast<int>((obj->position.y - worldMin_.y) / cellSize_);
    int z = static_cast<int>((obj->position.z - worldMin_.z) / cellSize_);

    // Clamp indices to grid bounds
    x = std::max(0, std::min(x, GRID_SIZE - 1));
    y = std::max(0, std::min(y, GRID_SIZE - 1));
    z = std::max(0, std::min(z, GRID_SIZE - 1));

    size_t index = getFlatIndex(x, y, z);



    if (index < grid_.size()) {
        grid_[index].push_back(obj);
    } else {
        printf("SPATIAL GRID ERROR: Index %zu out of bounds for grid size %zu\n", index, grid_.size());
    }
}

void EnvironmentManager::SpatialGrid::remove(std::shared_ptr<EnvironmentalObject> obj) {
    // Calculate 3D indices first
    int x = static_cast<int>((obj->position.x - worldMin_.x) / cellSize_);
    int y = static_cast<int>((obj->position.y - worldMin_.y) / cellSize_);
    int z = static_cast<int>((obj->position.z - worldMin_.z) / cellSize_);

    // Clamp indices to grid bounds
    x = std::max(0, std::min(x, GRID_SIZE - 1));
    y = std::max(0, std::min(y, GRID_SIZE - 1));
    z = std::max(0, std::min(z, GRID_SIZE - 1));

    size_t index = getFlatIndex(x, y, z);
    if (index < grid_.size()) {
        auto& cell = grid_[index];
        cell.erase(std::remove(cell.begin(), cell.end(), obj), cell.end());
    }
}

std::vector<std::shared_ptr<EnvironmentalObject>> EnvironmentManager::SpatialGrid::query(const BoundingBox& area) const {
    std::vector<std::shared_ptr<EnvironmentalObject>> results;

    // Calculate min/max cell indices with bounds checking
    int minX = std::max(0, static_cast<int>((area.min.x - worldMin_.x) / cellSize_));
    int minY = std::max(0, static_cast<int>((area.min.y - worldMin_.y) / cellSize_));
    int minZ = std::max(0, static_cast<int>((area.min.z - worldMin_.z) / cellSize_));
    int maxX = std::min(GRID_SIZE - 1, static_cast<int>((area.max.x - worldMin_.x) / cellSize_));
    int maxY = std::min(GRID_SIZE - 1, static_cast<int>((area.max.y - worldMin_.y) / cellSize_));
    int maxZ = std::min(GRID_SIZE - 1, static_cast<int>((area.max.z - worldMin_.z) / cellSize_));



    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                size_t index = getFlatIndex(x, y, z);
                if (index < grid_.size()) {
                    const auto& cellObjects = grid_[index];
                    results.insert(results.end(), cellObjects.begin(), cellObjects.end());
                } else {
                    printf("SPATIAL GRID QUERY ERROR: Index %zu out of bounds for grid size %zu\n", index, grid_.size());
                }
            }
        }
    }

    return results;
}

void EnvironmentManager::SpatialGrid::rebuild() {
    grid_.clear();
    grid_.resize(TOTAL_CELLS);
    cellSize_ = std::max({(worldMax_.x - worldMin_.x) / GRID_SIZE,
                          (worldMax_.y - worldMin_.y) / GRID_SIZE,
                          (worldMax_.z - worldMin_.z) / GRID_SIZE});
    hasBeenInitialized_ = true;
}

void EnvironmentManager::SpatialGrid::rebuildWithObjects(const std::vector<std::shared_ptr<EnvironmentalObject>>& objects) {
    rebuild();  // First rebuild the grid structure

    // Then insert all existing objects
    for (const auto& obj : objects) {
        if (obj->collidable) {  // Only insert collidable objects
            insert(obj);
        }
    }
}

// LODManager implementations
EnvironmentManager::DetailLevel EnvironmentManager::LODManager::getLODLevel(const Vector3& cameraPos, const Vector3& objectPos, float maxDistance) {
    float distance = MathUtils::distance3D(cameraPos, objectPos);
    if (distance > maxDistance) return DetailLevel::CULLED;
    if (distance > maxDistance * 0.7f) return DetailLevel::LOW;
    if (distance > maxDistance * 0.3f) return DetailLevel::MEDIUM;
    return DetailLevel::HIGH;
}

void EnvironmentManager::LODManager::updateLODLevels(const Camera3D& camera, std::vector<std::shared_ptr<EnvironmentalObject>>& objects) {
    // Update per-object LOD (objects could have a DetailLevel member or modify render based on this)
    // For now, placeholder - expand with object-specific LOD logic
    float maxDist = 50.0f;  // Adjust based on world
    for (auto& obj : objects) {
        getLODLevel(camera.position, obj->position, maxDist);
        // e.g., obj->setLOD(level);
    }
}

// AsyncLoader implementations
void EnvironmentManager::AsyncLoader::loadObjectAsync(const std::string& configPath, LoadCallback callback) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    loadQueue_.push({configPath, callback});
}

void EnvironmentManager::AsyncLoader::processCompletedLoads(EnvironmentManager* manager) {
    // Placeholder: Since loader is async, process completed in main thread
    // For simplicity, simulate loading here; in real, use condition variable for notification
    // Note: Actual loading logic depends on how objects are loaded from configPath (e.g., parse file, create object)
    std::lock_guard<std::mutex> lock(queueMutex_);
    while (!loadQueue_.empty()) {
        auto front = loadQueue_.front();
        std::string path = front.first;
        LoadCallback cb = front.second;
        loadQueue_.pop();
        // Simulate load (replace with real loading, e.g., parse JSON, create using factory)
        auto obj = EnvironmentalObjectFactory::createBuilding({/*config*/}, {0,0,0});  // Example
        cb(obj);
        manager->addObject(obj);
    }
}

void EnvironmentManager::AsyncLoader::loaderLoop() {
    while (running_) {
        // Wait for queue items, load in background
        // Use condition variable for efficiency
    }
}