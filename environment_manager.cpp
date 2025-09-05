#include "environment_manager.h"
#include "math_utils.h"
#include "raylib.h"
#include <cmath>
#include <iostream>  // For error logging

void EnvironmentManager::addObject(std::shared_ptr<EnvironmentalObject> obj) {
    objects_.push_back(obj);

    // Initialize spatial grid if not already done
    if (spatial_grid_.isEmpty()) {
        spatial_grid_.rebuild();
    }

    // Insert object into spatial grid if collidable
    if (obj->collidable) {
        spatial_grid_.insert(obj);
    }

    std::cout << "Added object: " << obj->getName() << " at (" << obj->position.x << ", " << obj->position.y << ", " << obj->position.z << "), collidable: " << obj->collidable << std::endl;
}

void EnvironmentManager::rebuildSpatialGrid() {
    std::cout << "ENVIRONMENT: Rebuilding spatial grid with " << objects_.size() << " objects" << std::endl;
    spatial_grid_.rebuildWithObjects(objects_);
}

void EnvironmentManager::update(float deltaTime, const Camera3D& camera) {
    for (auto& obj : objects_) {
        obj->update(deltaTime);
    }
    lod_manager_.updateLODLevels(camera, objects_);
    async_loader_.processCompletedLoads(this);
}

void EnvironmentManager::renderAll(const Camera3D& camera) {
    for (auto& obj : objects_) {
        obj->render(camera);
    }
}

bool EnvironmentManager::checkCollision(const CollisionBounds& bounds, int excludeIndex) const {
    // Use spatial query for candidates
    BoundingBox queryBox = CollisionSystem::boundsToBox(bounds);
    auto candidates = spatial_grid_.query(queryBox);
    
    for (const auto& obj : candidates) {
        if (obj->collidable) {
            // Check exclusion
            bool shouldExclude = false;
            if (excludeIndex != -1) {
                if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                    if (building->getId() == excludeIndex) {
                        shouldExclude = true;
                    }
                } else {
                    // Fallback to index
                    auto it = std::find(objects_.begin(), objects_.end(), obj);
                    if (it != objects_.end()) {
                        size_t idx = std::distance(objects_.begin(), it);
                        if (static_cast<int>(idx) == excludeIndex) {
                            shouldExclude = true;
                        }
                    }
                }
            }

            if (!shouldExclude) {
                auto objBounds = obj->getCollisionBounds();
                if (CollisionSystem::checkCollision(bounds, objBounds)) {
                    std::cout << "Collision detected with " << obj->getName() << std::endl;
                    return true;
                }
            }
        }
    }
    return false;
}

std::vector<std::shared_ptr<EnvironmentalObject>> EnvironmentManager::getInteractiveObjects() const {
    std::vector<std::shared_ptr<EnvironmentalObject>> interactive;
    for (const auto& obj : objects_) {
        if (obj->isInteractive()) {
            interactive.push_back(obj);
        }
    }
    return interactive;
}

const std::vector<std::shared_ptr<EnvironmentalObject>>& EnvironmentManager::getAllObjects() const {
    return objects_;
}

void EnvironmentManager::loadObjectAsync(const std::string& configPath, LoadCallback callback) {
    async_loader_.loadObjectAsync(configPath, callback);
}

// SpatialGrid
size_t EnvironmentManager::SpatialGrid::getFlatIndex(int x, int y, int z) const {
    return static_cast<size_t>(x * GRID_SIZE * GRID_SIZE + y * GRID_SIZE + z);
}

void EnvironmentManager::SpatialGrid::insert(std::shared_ptr<EnvironmentalObject> obj) {
    int x = static_cast<int>((obj->position.x - world_min_.x) / cell_size_);
    int y = static_cast<int>((obj->position.y - world_min_.y) / cell_size_);
    int z = static_cast<int>((obj->position.z - world_min_.z) / cell_size_);

    x = std::clamp(x, 0, GRID_SIZE - 1);
    y = std::clamp(y, 0, GRID_SIZE - 1);
    z = std::clamp(z, 0, GRID_SIZE - 1);

    size_t index = getFlatIndex(x, y, z);
    if (index < grid_.size()) {
        grid_[index].push_back(obj);
    } else {
        std::cerr << "Spatial grid index out of bounds: " << index << std::endl;
    }
}

void EnvironmentManager::SpatialGrid::remove(std::shared_ptr<EnvironmentalObject> obj) {
    int x = static_cast<int>((obj->position.x - world_min_.x) / cell_size_);
    int y = static_cast<int>((obj->position.y - world_min_.y) / cell_size_);
    int z = static_cast<int>((obj->position.z - world_min_.z) / cell_size_);

    x = std::clamp(x, 0, GRID_SIZE - 1);
    y = std::clamp(y, 0, GRID_SIZE - 1);
    z = std::clamp(z, 0, GRID_SIZE - 1);

    size_t index = getFlatIndex(x, y, z);
    if (index < grid_.size()) {
        auto& cell = grid_[index];
        cell.erase(std::remove(cell.begin(), cell.end(), obj), cell.end());
    }
}

std::vector<std::shared_ptr<EnvironmentalObject>> EnvironmentManager::SpatialGrid::query(const BoundingBox& area) const {
    std::vector<std::shared_ptr<EnvironmentalObject>> results;

    int minX = std::clamp(static_cast<int>((area.min.x - world_min_.x) / cell_size_), 0, GRID_SIZE - 1);
    int minY = std::clamp(static_cast<int>((area.min.y - world_min_.y) / cell_size_), 0, GRID_SIZE - 1);
    int minZ = std::clamp(static_cast<int>((area.min.z - world_min_.z) / cell_size_), 0, GRID_SIZE - 1);
    int maxX = std::clamp(static_cast<int>((area.max.x - world_min_.x) / cell_size_), 0, GRID_SIZE - 1);
    int maxY = std::clamp(static_cast<int>((area.max.y - world_min_.y) / cell_size_), 0, GRID_SIZE - 1);
    int maxZ = std::clamp(static_cast<int>((area.max.z - world_min_.z) / cell_size_), 0, GRID_SIZE - 1);

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                size_t index = getFlatIndex(x, y, z);
                if (index < grid_.size()) {
                    const auto& cell = grid_[index];
                    results.insert(results.end(), cell.begin(), cell.end());
                }
            }
        }
    }

    return results;
}

void EnvironmentManager::SpatialGrid::rebuild() {
    grid_.assign(TOTAL_CELLS, {});
    cell_size_ = std::max({(world_max_.x - world_min_.x) / GRID_SIZE,
                           (world_max_.y - world_min_.y) / GRID_SIZE,
                           (world_max_.z - world_min_.z) / GRID_SIZE});
    has_been_initialized_ = true;
}

void EnvironmentManager::SpatialGrid::rebuildWithObjects(const std::vector<std::shared_ptr<EnvironmentalObject>>& objects) {
    rebuild();
    for (const auto& obj : objects) {
        if (obj->collidable) {
            insert(obj);
        }
    }
}

bool EnvironmentManager::SpatialGrid::isEmpty() const {
    return !has_been_initialized_;
}

size_t EnvironmentManager::SpatialGrid::getGridSize() const {
    return grid_.size();
}

// LODManager
EnvironmentManager::DetailLevel EnvironmentManager::LODManager::getLODLevel(const Vector3& cameraPos, const Vector3& objectPos, float maxDistance) {
    float distance = MathUtils::distance3D(cameraPos, objectPos);
    if (distance > maxDistance) return DetailLevel::CULLED;
    if (distance > maxDistance * 0.7f) return DetailLevel::LOW;
    if (distance > maxDistance * 0.3f) return DetailLevel::MEDIUM;
    return DetailLevel::HIGH;
}

void EnvironmentManager::LODManager::updateLODLevels(const Camera3D& camera, std::vector<std::shared_ptr<EnvironmentalObject>>& objects) {
    float maxDist = 50.0f;  // Adjust based on world
    for (auto& obj : objects) {
        // obj->setLOD(getLODLevel(camera.position, obj->position, maxDist));
    }
}

// AsyncLoader
void EnvironmentManager::AsyncLoader::loadObjectAsync(const std::string& configPath, LoadCallback callback) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    load_queue_.push({configPath, callback});
}

void EnvironmentManager::AsyncLoader::processCompletedLoads(EnvironmentManager* manager) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!load_queue_.empty()) {
        auto [path, cb] = load_queue_.front();
        load_queue_.pop();
        // Simulate/actual load
        auto obj = std::make_shared<EnvironmentalObject>();  // Replace with actual factory
        cb(obj);
        manager->addObject(obj);
    }
}

void EnvironmentManager::AsyncLoader::loaderLoop() {
    while (running_) {
        // Process queue
    }
}