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
#include <unordered_map>

/// \brief Manages environmental objects with spatial partitioning and LOD.
class EnvironmentManager {
public:
    /// \brief Adds an object.
    /// \param obj Object to add.
    void addObject(std::shared_ptr<EnvironmentalObject> obj);

    /// \brief Updates all objects.
    /// \param deltaTime Delta time.
    /// \param camera Camera.
    void update(float deltaTime, const Camera3D& camera);

    /// \brief Renders all objects.
    /// \param camera Camera.
    void renderAll(const Camera3D& camera);

    /// \brief Rebuilds spatial grid.
    void rebuildSpatialGrid();

    /// \brief Checks collision.
    /// \param bounds Bounds to check.
    /// \param excludeIndex Index to exclude.
    /// \return True if colliding.
    bool checkCollision(const CollisionBounds& bounds, int excludeIndex = -1) const;

    /// \brief Gets interactive objects.
    /// \return Interactive objects.
    std::vector<std::shared_ptr<EnvironmentalObject>> getInteractiveObjects() const;

    /// \brief Gets all objects.
    /// \return All objects.
    const std::vector<std::shared_ptr<EnvironmentalObject>>& getAllObjects() const;

    // New: Async loading
    using LoadCallback = std::function<void(std::shared_ptr<EnvironmentalObject>)>;
    /// \brief Loads object async.
    /// \param configPath Config path.
    /// \param callback Callback on load.
    void loadObjectAsync(const std::string& configPath, LoadCallback callback);

private:
    std::vector<std::shared_ptr<EnvironmentalObject>> objects_;

    // New: Spatial partitioning
    class SpatialGrid {
    public:
        /// \brief Inserts object.
        /// \param obj Object to insert.
        void insert(std::shared_ptr<EnvironmentalObject> obj);

        /// \brief Removes object.
        /// \param obj Object to remove.
        void remove(std::shared_ptr<EnvironmentalObject> obj);

        /// \brief Queries area.
        /// \param area Area to query.
        /// \return Objects in area.
        std::vector<std::shared_ptr<EnvironmentalObject>> query(const BoundingBox& area) const;

        /// \brief Rebuilds grid.
        void rebuild();

        /// \brief Rebuilds with objects.
        /// \param objects Objects to insert.
        void rebuildWithObjects(const std::vector<std::shared_ptr<EnvironmentalObject>>& objects);

        /// \brief Checks if empty.
        /// \return True if empty.
        bool isEmpty() const;

        /// \brief Gets grid size.
        /// \return Grid size.
        size_t getGridSize() const;

    private:
        static constexpr int GRID_SIZE = 64;
        static constexpr size_t TOTAL_CELLS = GRID_SIZE * GRID_SIZE * GRID_SIZE;
        std::vector<std::vector<std::shared_ptr<EnvironmentalObject>>> grid_;
        Vector3 world_min_ = {-50, -10, -50};  // Smaller bounds for actual game world
        Vector3 world_max_ = {50, 10, 50};
        float cell_size_;
        size_t getFlatIndex(int x, int y, int z) const;
        bool has_been_initialized_ = false;
    };
    SpatialGrid spatial_grid_;

    // New: LOD System
    enum class DetailLevel { HIGH, MEDIUM, LOW, CULLED };

    class LODManager {
    public:
        /// \brief Gets LOD level.
        /// \param cameraPos Camera position.
        /// \param objectPos Object position.
        /// \param maxDistance Max distance.
        /// \return LOD level.
        DetailLevel getLODLevel(const Vector3& cameraPos, const Vector3& objectPos, float maxDistance);

        /// \brief Updates LOD levels.
        /// \param camera Camera.
        /// \param objects Objects.
        void updateLODLevels(const Camera3D& camera, std::vector<std::shared_ptr<EnvironmentalObject>>& objects);
    };
    LODManager lod_manager_;

    // New: Async Loader
    class AsyncLoader {
    public:
        using LoadCallback = std::function<void(std::shared_ptr<EnvironmentalObject>)>;

        /// \brief Loads object async.
        /// \param configPath Config path.
        /// \param callback Callback.
        void loadObjectAsync(const std::string& configPath, LoadCallback callback);

        /// \brief Processes completed loads.
        /// \param manager Environment manager.
        void processCompletedLoads(EnvironmentManager* manager);

    private:
        std::queue<std::pair<std::string, LoadCallback>> load_queue_;
        std::mutex queue_mutex_;
        std::thread loader_thread_;
        bool running_ = true;
        void loaderLoop();
    };
    AsyncLoader async_loader_;
};

#endif