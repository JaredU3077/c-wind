// world_builder.cpp
#include "world_builder.h"
#include "environmental_object.h"

void initializeWorld(EnvironmentManager& environment) {
    // Create Mayor's Building (north side, blue, facing player spawn)
    Building::DoorConfig mayorDoor;
    mayorDoor.offset = {0.0f, -2.5f, 4.0f};   // Door on north face (+Z)
    mayorDoor.width = 1.2f;
    mayorDoor.height = 2.8f;
    mayorDoor.rotation = 0.0f;
    mayorDoor.color = BROWN;

    auto mayorBuilding = std::make_shared<Building>(
        Vector3{0.0f, 2.5f, -12.0f},    // Building center
        Vector3{10.0f, 5.0f, 8.0f},    // Size
        BLUE,                           // Color
        "Mayor's Building",             // Name
        mayorDoor,                      // Door config
        true                            // Can enter
    );
    environment.addObject(mayorBuilding);

    // Create Shop Keeper's Building (east side, red, facing player spawn)
    Building::DoorConfig shopDoor;
    shopDoor.offset = {4.0f, -2.5f, 0.0f};    // Door on east face (+X)
    shopDoor.width = 1.2f;
    shopDoor.height = 2.8f;
    shopDoor.rotation = 90.0f; // Rotate to align properly
    shopDoor.color = BROWN;

    auto shopBuilding = std::make_shared<Building>(
        Vector3{12.0f, 2.5f, 0.0f},     // Building center (consistent height)
        Vector3{8.0f, 5.0f, 6.0f},      // Size (consistent height)
        RED,                            // Color
        "Shop Keeper's Building",       // Name
        shopDoor,                       // Door config
        true                            // Can enter
    );
    environment.addObject(shopBuilding);

    // Add central well
    auto well = std::make_shared<Well>(
        Vector3{0.0f, 0.0f, 0.0f},  // Position
        1.5f,                       // Base radius
        2.5f                        // Height
    );
    environment.addObject(well);

    // Add decorative trees
    auto tree1 = std::make_shared<Tree>(
        Vector3{-15.0f, 0.0f, -10.0f},  // Position
        0.5f,                           // Trunk radius
        4.0f,                           // Trunk height
        2.5f                            // Foliage radius
    );
    environment.addObject(tree1);

    auto tree2 = std::make_shared<Tree>(
        Vector3{15.0f, 0.0f, -10.0f},   // Position
        0.5f,                           // Trunk radius
        4.0f,                           // Trunk height
        2.5f                            // Foliage radius
    );
    environment.addObject(tree2);
}