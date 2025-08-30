// environment_manager.h (updated)
#ifndef ENVIRONMENT_MANAGER_H
#define ENVIRONMENT_MANAGER_H

#include "collision_system.h"
#include "environmental_object.h"
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>

class EnvironmentManager {
public:
    void addObject(std::shared_ptr<EnvironmentalObject> obj);
    void update(float deltaTime, const Camera3D& camera);
    void renderAll(const Camera3D& camera);
    void rebuildSpatialGrid();  // Rebuild spatial grid with all current objects
    bool checkCollision(const CollisionBounds& bounds, int excludeIndex = -1) const;
    std::vector<std::shared_ptr<EnvironmentalObject>> getInteractiveObjects() const;
    const std::vector<std::shared_ptr<EnvironmentalObject>>& getAllObjects() const;

    // New: Async loading
    using LoadCallback = std::function<void(std::shared_ptr<EnvironmentalObject>)>;
    void loadObjectAsync(const std::string& configPath, LoadCallback callback);

private:
    std::vector<std::shared_ptr<EnvironmentalObject>> objects;

    // New: Spatial partitioning
    class SpatialGrid {
    public:
        void insert(std::shared_ptr<EnvironmentalObject> obj);
        void remove(std::shared_ptr<EnvironmentalObject> obj);
        std::vector<std::shared_ptr<EnvironmentalObject>> query(const BoundingBox& area) const;
        void rebuild();
        void rebuildWithObjects(const std::vector<std::shared_ptr<EnvironmentalObject>>& objects);
        bool isEmpty() const { return !hasBeenInitialized_; }
        size_t getGridSize() const { return grid_.size(); }

    private:
        static constexpr int GRID_SIZE = 64;
        static constexpr size_t TOTAL_CELLS = GRID_SIZE * GRID_SIZE * GRID_SIZE;
        std::vector<std::vector<std::shared_ptr<EnvironmentalObject>>> grid_;
        Vector3 worldMin_ = {-50, -10, -50};  // Smaller bounds for actual game world
        Vector3 worldMax_ = {50, 10, 50};
        float cellSize_;
        size_t getFlatIndex(int x, int y, int z) const;
        bool hasBeenInitialized_ = false;
    };
    SpatialGrid spatialGrid_;

    // New: LOD System
    enum class DetailLevel { HIGH, MEDIUM, LOW, CULLED };

    class LODManager {
    public:
        DetailLevel getLODLevel(const Vector3& cameraPos, const Vector3& objectPos, float maxDistance);
        void updateLODLevels(const Camera3D& camera, std::vector<std::shared_ptr<EnvironmentalObject>>& objects);
    };
    LODManager lodManager_;

    // New: Async Loader
    class AsyncLoader {
    public:
        using LoadCallback = std::function<void(std::shared_ptr<EnvironmentalObject>)>;

        void loadObjectAsync(const std::string& configPath, LoadCallback callback);
        void processCompletedLoads(EnvironmentManager* manager);

    private:
        std::queue<std::pair<std::string, LoadCallback>> loadQueue_;
        std::mutex queueMutex_;
        std::thread loaderThread_;
        bool running_ = true;
        void loaderLoop();
    };
    AsyncLoader asyncLoader_;
};

#endif