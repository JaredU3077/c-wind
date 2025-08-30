// world_builder.cpp
#include "world_builder.h"
#include "environmental_object.h"

void initializeWorld(EnvironmentManager& environment) {
    // Create Mayor's Building (north side, blue, facing player spawn)
    DoorConfig mayorDoor;
    mayorDoor.offset = {0.0f, -2.5f, 4.0f};   // Door on north face (+Z)
    mayorDoor.width = 1.2f;
    mayorDoor.height = 2.8f;
    mayorDoor.rotation = 0.0f;
    mayorDoor.color = BROWN;

    BuildingConfig mayorConfig;
    mayorConfig.id = 0;  // Unique ID for Mayor's Building
    mayorConfig.size = {10.0f, 5.0f, 8.0f};
    mayorConfig.color = BLUE;
    mayorConfig.name = "Mayor's Building";
    mayorConfig.door = mayorDoor;
    mayorConfig.canEnter = true;

    auto mayorBuilding = EnvironmentalObjectFactory::createBuilding(mayorConfig, Vector3{0.0f, 2.5f, -12.0f});
    environment.addObject(mayorBuilding);

    // Create Shop Keeper's Building (east side, red, facing player spawn)
    DoorConfig shopDoor;
    shopDoor.offset = {4.0f, -2.5f, 0.0f};    // Door on east face (+X)
    shopDoor.width = 1.2f;
    shopDoor.height = 2.8f;
    shopDoor.rotation = 90.0f; // Rotate to align properly
    shopDoor.color = BROWN;

    BuildingConfig shopConfig;
    shopConfig.id = 1;  // Unique ID for Shop Building
    shopConfig.size = {8.0f, 5.0f, 6.0f};
    shopConfig.color = RED;
    shopConfig.name = "Shop Keeper's Building";
    shopConfig.door = shopDoor;
    shopConfig.canEnter = true;

    auto shopBuilding = EnvironmentalObjectFactory::createBuilding(shopConfig, Vector3{12.0f, 2.5f, 0.0f});
    environment.addObject(shopBuilding);

    // Add central well
    WellConfig wellConfig;
    wellConfig.baseRadius = 1.5f;
    wellConfig.height = 2.5f;

    auto well = EnvironmentalObjectFactory::createWell(wellConfig, Vector3{0.0f, 0.0f, 0.0f});
    environment.addObject(well);

    // Add decorative trees
    TreeConfig treeConfig;
    treeConfig.trunkRadius = 0.5f;
    treeConfig.trunkHeight = 4.0f;
    treeConfig.foliageRadius = 2.5f;

    auto tree1 = EnvironmentalObjectFactory::createTree(treeConfig, Vector3{-15.0f, 0.0f, -10.0f});
    environment.addObject(tree1);

    auto tree2 = EnvironmentalObjectFactory::createTree(treeConfig, Vector3{15.0f, 0.0f, -10.0f});
    environment.addObject(tree2);

    // Rebuild spatial grid with all objects for proper collision detection
    environment.rebuildSpatialGrid();
}