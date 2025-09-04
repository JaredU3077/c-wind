// world_builder.cpp - Complete town world initialization
#include "world_builder.h"
#include "environmental_object.h"
#include "constants.h"
#include <iostream>
#include <memory>

void initializeWorld(EnvironmentManager& environment) {
    try {
        std::cout << "WorldBuilder: Starting complete town world initialization..." << std::endl;

        // ===== CREATE MAYOR'S BUILDING (ID 0) =====
        BuildingConfig mayorBuilding = {
            .id = 0,
            .size = {10.0f, 5.0f, 8.0f},
            .color = BLUE,
            .name = "Mayor's Office",
            .door = {
                .offset = {0.0f, 0.0f, -4.1f},
                .width = EnvironmentConstants::BUILDING_DOOR_WIDTH,
                .height = EnvironmentConstants::BUILDING_DOOR_HEIGHT,
                .rotation = 0.0f,
                .color = BROWN
            },
            .canEnter = true
        };

        auto mayorBuildingObj = EnvironmentalObjectFactory::createBuilding(mayorBuilding, {-12.0f, 2.5f, 0.0f});
        environment.addObject(mayorBuildingObj);
        std::cout << "WorldBuilder: Created Mayor's Office building" << std::endl;

        // ===== CREATE SHOP BUILDING (ID 1) =====
        BuildingConfig shopBuilding = {
            .id = 1,
            .size = {8.0f, 5.0f, 6.0f},
            .color = RED,
            .name = "General Store",
            .door = {
                .offset = {0.0f, 0.0f, 3.1f},
                .width = EnvironmentConstants::BUILDING_DOOR_WIDTH,
                .height = EnvironmentConstants::BUILDING_DOOR_HEIGHT,
                .rotation = 0.0f,
                .color = BROWN
            },
            .canEnter = true
        };

        auto shopBuildingObj = EnvironmentalObjectFactory::createBuilding(shopBuilding, {0.0f, 2.5f, 12.0f});
        environment.addObject(shopBuildingObj);
        std::cout << "WorldBuilder: Created General Store building" << std::endl;

        // ===== CREATE CENTRAL WELL =====
        WellConfig wellConfig = {
            .baseRadius = EnvironmentConstants::WELL_BASE_RADIUS,
            .height = EnvironmentConstants::WELL_HEIGHT
        };

        auto wellObj = EnvironmentalObjectFactory::createWell(wellConfig, {0.0f, wellConfig.height/2, 0.0f});  // Position so base is at ground level
        environment.addObject(wellObj);
        std::cout << "WorldBuilder: Created Central Well" << std::endl;

        // ===== CREATE TREES AROUND THE TOWN =====
        TreeConfig treeConfig = {
            .trunkRadius = EnvironmentConstants::TREE_TRUNK_RADIUS,
            .trunkHeight = EnvironmentConstants::TREE_TRUNK_HEIGHT,
            .foliageRadius = EnvironmentConstants::TREE_FOLIAGE_RADIUS
        };

        // Create several trees around the town perimeter - closer for visibility
        // Position trees so their base is at ground level (Y=0.0f)
        // Camera starts at (0, 1.75, 5) looking toward (0, 1.75, 0)
        std::vector<Vector3> treePositions = {
            {-8.0f, 0.0f, -8.0f},   // Closer to camera for visibility
            {8.0f, 0.0f, -8.0f},
            {-8.0f, 0.0f, 8.0f},
            {8.0f, 0.0f, 8.0f},
            {-12.0f, 0.0f, 0.0f},   // Along the sides
            {12.0f, 0.0f, 0.0f}
        };

        for (size_t i = 0; i < treePositions.size(); ++i) {
            auto treeObj = EnvironmentalObjectFactory::createTree(treeConfig, treePositions[i]);
            environment.addObject(treeObj);
        }
        std::cout << "WorldBuilder: Created " << treePositions.size() << " trees around town" << std::endl;

        std::cout << "WorldBuilder: Complete town world initialization finished successfully!" << std::endl;
        std::cout << "WorldBuilder: Created buildings, well, and environmental objects" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "WorldBuilder: Exception during world initialization: " << e.what() << std::endl;
        std::cout << "WorldBuilder: Continuing with minimal world setup" << std::endl;
    }
}