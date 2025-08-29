// Browserwind.cpp
// A very simple minimal working vertical slice of a 3D FPS like
// .
// This uses raylib (https://www.raylib.com/) for simplicity in handling window, input, and 3D rendering.
// Compile with: g++ Browserwind.cpp -o Browserwind -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 (adjust for your platform)
// Assumes raylib is installed and linked properly.
// Features: First-person camera movement (WASD to move, mouse to look, space to jump), simple 3D scene with floor and a cube.

#include "raylib.h"
#include "rlgl.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Forward declarations for dialog system
void startDialog(int npcIndex);
void handleDialogOption(int optionIndex);

// Global dialog state variables
bool isInDialog = false;
int currentNPC = -1;
std::string dialogText = "";
std::string dialogOptions[3] = {"", "", ""};
int numDialogOptions = 0;
bool showDialogWindow = false;
bool mouseReleased = false; // Track if mouse was intentionally released with ESC

// ===== UNIVERSAL COLLISION SYSTEM =====
// Base collision shape interface
enum class CollisionShape {
    BOX,        // Axis-aligned bounding box
    SPHERE,     // Sphere collision
    CYLINDER,   // Cylinder collision
    CAPSULE     // Capsule collision (for characters)
};

// Universal collision bounds structure
struct CollisionBounds {
    CollisionShape shape;
    Vector3 position;
    Vector3 size;      // For box: width/height/depth, for sphere: radius, for cylinder: radius/height
    float rotation;    // Rotation around Y axis (for oriented collision)
};

// ===== ENVIRONMENTAL OBJECT SYSTEM =====
// Base class for all environmental objects
class EnvironmentalObject {
public:
    virtual ~EnvironmentalObject() = default;

    virtual void render() = 0;
    virtual CollisionBounds getCollisionBounds() const = 0;
    virtual bool isInteractive() const { return false; }
    virtual void interact() {}
    virtual std::string getName() const { return "Object"; }
    virtual float getInteractionRadius() const { return 0.0f; }

    Vector3 position;
    bool collidable = true;
};

// Building system with standardized door mounting
class Building : public EnvironmentalObject {
public:
    struct DoorConfig {
        Vector3 offset;        // Offset from building center
        float width = 1.2f;
        float height = 2.8f;
        float rotation = 0.0f; // Rotation around Y axis
        Color color = BROWN;
    };

    Building(Vector3 pos, Vector3 size, Color color, const std::string& name,
             const DoorConfig& door, bool canEnter = true)
        : size(size), color(color), name(name), doorConfig(door), canEnter(canEnter) {
        position = pos;
    }

    void render() override {
        // Draw main building
        DrawCube(position, size.x, size.y, size.z, color);
        DrawCubeWires(position, size.x, size.y, size.z, BLACK);

        // Draw roof
        Vector3 roofPos = {position.x, position.y + size.y/2 + 0.5f, position.z};
        DrawCube(roofPos, size.x + 1.0f, 1.0f, size.z + 1.0f, DARKGRAY);

        // Draw properly mounted door with rotation support
        Vector3 doorPos = {
            position.x + doorConfig.offset.x,
            position.y + doorConfig.offset.y,
            position.z + doorConfig.offset.z
        };

        rlPushMatrix();
        rlTranslatef(doorPos.x, doorPos.y + doorConfig.height / 2.0f, doorPos.z); // Center for rotation
        rlRotatef(doorConfig.rotation, 0.0f, 1.0f, 0.0f);
        DrawCube({0, 0, 0}, doorConfig.width, doorConfig.height, 0.2f, doorConfig.color);
        DrawCubeWires({0, 0, 0}, doorConfig.width, doorConfig.height, 0.2f, BLACK);
        rlPopMatrix();

        // Draw door handle (adjusted for rotation)
        float handleX = cosf(doorConfig.rotation * DEG2RAD) * 0.4f;
        float handleZ = sinf(doorConfig.rotation * DEG2RAD) * 0.4f;
        Vector3 handlePos = {
            doorPos.x + handleX,
            doorPos.y + 1.0f,
            doorPos.z + handleZ
        };
        DrawSphere(handlePos, 0.08f, GOLD);

        // Draw building sign (rotated if necessary)
        Vector3 signPos = {doorPos.x, doorPos.y + 2.2f, doorPos.z};
        rlPushMatrix();
        rlTranslatef(signPos.x, signPos.y, signPos.z);
        rlRotatef(doorConfig.rotation, 0.0f, 1.0f, 0.0f);
        DrawCube({0, 0, 0}, 1.5f, 0.3f, 0.05f, LIGHTGRAY);
        rlPopMatrix();
    }

    CollisionBounds getCollisionBounds() const override {
        return {
            CollisionShape::BOX,
            position,
            {size.x, size.y, size.z},
            0.0f
        };
    }

    bool isInteractive() const override { return canEnter; }
    void interact() override {
        // Building entry logic would go here
    }

    std::string getName() const override { return name; }
    float getInteractionRadius() const override { return 3.0f; }

    // Door-specific methods
    Vector3 getDoorPosition() const {
        return {
            position.x + doorConfig.offset.x,
            position.y + doorConfig.offset.y,
            position.z + doorConfig.offset.z
        };
    }

    // Public getters for interior rendering
    Vector3 getSize() const { return size; }
    Color getColor() const { return color; }

    bool isPlayerAtDoor(Vector3 playerPos, float threshold = 3.0f) const {
        Vector3 doorPos = getDoorPosition();
        float distance = sqrtf(
            (playerPos.x - doorPos.x) * (playerPos.x - doorPos.x) +
            (playerPos.y - doorPos.y) * (playerPos.y - doorPos.y) +
            (playerPos.z - doorPos.z) * (playerPos.z - doorPos.z)
        );
        return distance <= threshold;
    }

private:
    Vector3 size;
    Color color;
    std::string name;
    DoorConfig doorConfig;
    bool canEnter;
};

// Universal well/fountain object
class Well : public EnvironmentalObject {
public:
    Well(Vector3 pos, float baseRadius = 1.5f, float height = 2.5f)
        : baseRadius(baseRadius), height(height) {
        position = pos;
    }

    void render() override {
        // Enhanced well rendering
        Vector3 wellBase = position;
        Vector3 wellTop = {position.x, position.y + height, position.z};

        // Well base (stone foundation)
        DrawCylinder(wellBase, baseRadius, baseRadius, 0.5f, 16, DARKGRAY);

        // Well walls (stone/brick)
        DrawCylinder(wellBase, baseRadius * 0.8f, baseRadius * 0.8f, height, 16, GRAY);

        // Well top rim
        DrawCylinder(wellTop, baseRadius * 0.9f, baseRadius * 0.7f, 0.2f, 16, DARKGRAY);

        // Well bucket mechanism
        DrawCylinder({position.x, position.y + height + 0.5f, position.z}, 0.1f, 0.1f, 1.0f, 8, BROWN);
        DrawSphere({position.x, position.y + height + 1.5f, position.z}, 0.15f, GRAY);
    }

    CollisionBounds getCollisionBounds() const override {
        return {
            CollisionShape::CYLINDER,
            position,
            {baseRadius, height, 0.0f},
            0.0f
        };
    }

    std::string getName() const override { return "Town Well"; }

private:
    float baseRadius;
    float height;
};

// Tree object
class Tree : public EnvironmentalObject {
public:
    Tree(Vector3 pos, float trunkRadius = 0.5f, float trunkHeight = 4.0f, float foliageRadius = 2.5f)
        : trunkRadius(trunkRadius), trunkHeight(trunkHeight), foliageRadius(foliageRadius) {
        position = pos;
    }

    void render() override {
        // Tree trunk
        DrawCylinder(position, trunkRadius, trunkRadius, trunkHeight, 8, DARKBROWN);

        // Tree foliage
        Vector3 foliagePos = {position.x, position.y + trunkHeight, position.z};
        DrawSphere(foliagePos, foliageRadius, GREEN);
    }

    CollisionBounds getCollisionBounds() const override {
        return {
            CollisionShape::CYLINDER,
            position,
            {trunkRadius, trunkHeight, 0.0f},
            0.0f
        };
    }

    std::string getName() const override { return "Tree"; }

private:
    float trunkRadius;
    float trunkHeight;
    float foliageRadius;
};

// ===== UNIVERSAL COLLISION DETECTION SYSTEM =====
class CollisionSystem {
public:
    static bool checkCollision(const CollisionBounds& bounds1, const CollisionBounds& bounds2) {
        // Universal collision detection between any two collision bounds
        switch (bounds1.shape) {
            case CollisionShape::BOX:
                return checkBoxCollision(bounds1, bounds2);
            case CollisionShape::SPHERE:
                return checkSphereCollision(bounds1, bounds2);
            case CollisionShape::CYLINDER:
                return checkCylinderCollision(bounds1, bounds2);
            case CollisionShape::CAPSULE:
                return checkCapsuleCollision(bounds1, bounds2);
            default:
                return false;
        }
    }

    static bool checkPointInBounds(Vector3 point, const CollisionBounds& bounds) {
        switch (bounds.shape) {
            case CollisionShape::BOX:
                return pointInBox(point, bounds);
            case CollisionShape::SPHERE:
                return pointInSphere(point, bounds);
            case CollisionShape::CYLINDER:
                return pointInCylinder(point, bounds);
            case CollisionShape::CAPSULE:
                return pointInCapsule(point, bounds);
            default:
                return false;
        }
    }

private:
    static bool checkBoxCollision(const CollisionBounds& box1, const CollisionBounds& bounds2) {
        float halfWidth1 = box1.size.x / 2.0f;
        float halfHeight1 = box1.size.y / 2.0f;
        float halfDepth1 = box1.size.z / 2.0f;

        float minX1 = box1.position.x - halfWidth1;
        float maxX1 = box1.position.x + halfWidth1;
        float minY1 = box1.position.y - halfHeight1;
        float maxY1 = box1.position.y + halfHeight1;
        float minZ1 = box1.position.z - halfDepth1;
        float maxZ1 = box1.position.z + halfDepth1;

        switch (bounds2.shape) {
            case CollisionShape::BOX: {
                float halfWidth2 = bounds2.size.x / 2.0f;
                float halfHeight2 = bounds2.size.y / 2.0f;
                float halfDepth2 = bounds2.size.z / 2.0f;

                float minX2 = bounds2.position.x - halfWidth2;
                float maxX2 = bounds2.position.x + halfWidth2;
                float minY2 = bounds2.position.y - halfHeight2;
                float maxY2 = bounds2.position.y + halfHeight2;
                float minZ2 = bounds2.position.z - halfDepth2;
                float maxZ2 = bounds2.position.z + halfDepth2;

                return (maxX1 >= minX2 && minX1 <= maxX2) &&
                       (maxY1 >= minY2 && minY1 <= maxY2) &&
                       (maxZ1 >= minZ2 && minZ1 <= maxZ2);
            }
            case CollisionShape::SPHERE: {
                // Box-sphere collision
                float closestX = std::max(minX1, std::min(bounds2.position.x, maxX1));
                float closestY = std::max(minY1, std::min(bounds2.position.y, maxY1));
                float closestZ = std::max(minZ1, std::min(bounds2.position.z, maxZ1));

                float distanceX = bounds2.position.x - closestX;
                float distanceY = bounds2.position.y - closestY;
                float distanceZ = bounds2.position.z - closestZ;

                return (distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ) <= (bounds2.size.x * bounds2.size.x);
            }
            case CollisionShape::CYLINDER: {
                // Simplified box-cylinder collision
                return (bounds2.position.x >= minX1 && bounds2.position.x <= maxX1) &&
                       (bounds2.position.y >= minY1 && bounds2.position.y <= maxY1) &&
                       (bounds2.position.z >= minZ1 && bounds2.position.z <= maxZ1);
            }
            default:
                return false;
        }
    }

    static bool checkSphereCollision(const CollisionBounds& sphere1, const CollisionBounds& bounds2) {
        float distance = sqrtf(
            (sphere1.position.x - bounds2.position.x) * (sphere1.position.x - bounds2.position.x) +
            (sphere1.position.y - bounds2.position.y) * (sphere1.position.y - bounds2.position.y) +
            (sphere1.position.z - bounds2.position.z) * (sphere1.position.z - bounds2.position.z)
        );

        switch (bounds2.shape) {
            case CollisionShape::SPHERE:
                return distance <= (sphere1.size.x + bounds2.size.x);
            case CollisionShape::CYLINDER:
                // Sphere-cylinder distance check
                return distance <= (sphere1.size.x + bounds2.size.x);
            default:
                return false;
        }
    }

    static bool checkCylinderCollision(const CollisionBounds& cyl1, const CollisionBounds& bounds2) {
        // Simplified cylinder collision - check if centers are close enough
        float distance = sqrtf(
            (cyl1.position.x - bounds2.position.x) * (cyl1.position.x - bounds2.position.x) +
            (cyl1.position.z - bounds2.position.z) * (cyl1.position.z - bounds2.position.z)
        );

        switch (bounds2.shape) {
            case CollisionShape::CYLINDER:
                return distance <= (cyl1.size.x + bounds2.size.x);
            case CollisionShape::SPHERE:
                return distance <= (cyl1.size.x + bounds2.size.x);
            case CollisionShape::BOX: {
                // Cylinder vs Box collision (vertical cylinder vs AABB)
                float halfWidth = bounds2.size.x / 2.0f;
                float halfHeight = bounds2.size.y / 2.0f;
                float halfDepth = bounds2.size.z / 2.0f;

                float minX = bounds2.position.x - halfWidth;
                float maxX = bounds2.position.x + halfWidth;
                float minY = bounds2.position.y - halfHeight;
                float maxY = bounds2.position.y + halfHeight;
                float minZ = bounds2.position.z - halfDepth;
                float maxZ = bounds2.position.z + halfDepth;

                // Closest point on box to cylinder axis in XZ
                float closestX = std::max(minX, std::min(cyl1.position.x, maxX));
                float closestZ = std::max(minZ, std::min(cyl1.position.z, maxZ));

                float dx = closestX - cyl1.position.x;
                float dz = closestZ - cyl1.position.z;
                float dist2D = sqrtf(dx * dx + dz * dz);

                if (dist2D > cyl1.size.x) return false;

                // Check vertical overlap
                float cylBottom = cyl1.position.y;
                float cylTop = cyl1.position.y + cyl1.size.y;
                return (cylTop > minY && cylBottom < maxY);
            }
            default:
                return false;
        }
    }

    static bool checkCapsuleCollision(const CollisionBounds& capsule1, const CollisionBounds& bounds2) {
        // Capsule collision - approximated as cylinder for simplicity
        return checkCylinderCollision(capsule1, bounds2);
    }

    // Helper functions for point-in-bounds checks
    static bool pointInBox(Vector3 point, const CollisionBounds& box) {
        float halfWidth = box.size.x / 2.0f;
        float halfHeight = box.size.y / 2.0f;
        float halfDepth = box.size.z / 2.0f;

        return (point.x >= box.position.x - halfWidth && point.x <= box.position.x + halfWidth) &&
               (point.y >= box.position.y - halfHeight && point.y <= box.position.y + halfHeight) &&
               (point.z >= box.position.z - halfDepth && point.z <= box.position.z + halfDepth);
    }

    static bool pointInSphere(Vector3 point, const CollisionBounds& sphere) {
        float distance = sqrtf(
            (point.x - sphere.position.x) * (point.x - sphere.position.x) +
            (point.y - sphere.position.y) * (point.y - sphere.position.y) +
            (point.z - sphere.position.z) * (point.z - sphere.position.z)
        );
        return distance <= sphere.size.x;
    }

    static bool pointInCylinder(Vector3 point, const CollisionBounds& cylinder) {
        // Check height
        if (point.y < cylinder.position.y || point.y > cylinder.position.y + cylinder.size.y) {
            return false;
        }

        // Check radius
        float distance = sqrtf(
            (point.x - cylinder.position.x) * (point.x - cylinder.position.x) +
            (point.z - cylinder.position.z) * (point.z - cylinder.position.z)
        );
        return distance <= cylinder.size.x;
    }

    static bool pointInCapsule(Vector3 point, const CollisionBounds& capsule) {
        return pointInCylinder(point, capsule);
    }
};

// Environment manager to handle all environmental objects
class EnvironmentManager {
public:
    void addObject(std::shared_ptr<EnvironmentalObject> obj) {
        objects.push_back(obj);
    }

    void renderAll() {
        for (auto& obj : objects) {
            obj->render();
        }
    }

    bool checkCollision(const CollisionBounds& bounds, int excludeIndex = -1) const {
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

    std::vector<std::shared_ptr<EnvironmentalObject>> getInteractiveObjects() const {
        std::vector<std::shared_ptr<EnvironmentalObject>> interactive;
        for (const auto& obj : objects) {
            if (obj->isInteractive()) {
                interactive.push_back(obj);
            }
        }
        return interactive;
    }

    const std::vector<std::shared_ptr<EnvironmentalObject>>& getAllObjects() const {
        return objects;
    }

private:
    std::vector<std::shared_ptr<EnvironmentalObject>> objects;
};

int main(void)
{
    // Initialization
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Browserwind - 3D FPS Game (1920x1080)");

    // Define the camera
    Camera3D camera = {
        .position = (Vector3){ 4.0f, 1.75f, 4.0f },   // Camera starting position (eye height)
        .target = (Vector3){ 0.0f, 1.55f, 0.0f },     // Camera looking at point
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },         // Camera up vector (rotation towards target)
        .fovy = 60.0f,                               // Camera field-of-view Y
        .projection = CAMERA_PERSPECTIVE              // Camera projection type
    };

    // Hide and capture mouse cursor for FPS gameplay
    DisableCursor(); // Mouse starts captured
    mouseReleased = false; // Ensure mouse is captured at start

    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second

    // Jumping system variables
    float playerY = 0.0f; // Player's bottom Y position
    const float eyeHeight = 1.75f;
    bool isJumping = false;
    bool isGrounded = true;
    float jumpVelocity = 0.0f;
    const float jumpStrength = 8.0f;     // Jump power
    const float gravity = -15.0f;        // Gravity strength
    const float groundLevel = 0.0f;      // Ground Y position

    // Melee combat system variables
    struct LongswordSwing {
        Vector3 startPosition;
        Vector3 endPosition;
        Vector3 direction;
        bool active;
        float progress; // 0.0 to 1.0 (swing animation progress)
        float lifetime;
    };
    const int MAX_SWINGS = 3;
    LongswordSwing swings[MAX_SWINGS];
    float lastSwingTime = 0.0f;
    const float swingCooldown = 0.5f; // 500ms between swings
    const float swingRange = 3.0f;    // Melee range
    const float swingSpeed = 8.0f;    // Swing animation speed
    const float swingDuration = 0.3f; // How long swing lasts

    // ===== NEW ENVIRONMENTAL OBJECT SYSTEM =====
    EnvironmentManager environment;

    // Player collision bounds (capsule shape)
    const float PLAYER_RADIUS = 0.4f;
    const float PLAYER_HEIGHT = 1.8f;

    // NPC system variables
    struct NPC {
        Vector3 position;
        std::string name;
        std::string dialog;
        Color color;
        bool canInteract;
        float interactionRadius;
    };
    const int MAX_NPCS = 2;
    NPC npcs[MAX_NPCS];

    // Game state variables
    // Game state variables
    bool isInBuilding = false;
    int currentBuilding = -1;
    bool showInteractPrompt = false;
    std::string interactPromptText = "";
    Vector3 lastOutdoorPosition = {0.0f, 1.75f, 0.0f};
    bool testBuildingEntry = false;



    // Target system variables (keeping for combat)
    struct Target {
        Vector3 position;
        bool active;
        bool hit;
        float hitTime;
        Color color;
    };
    const int MAX_TARGETS = 5;
    Target targets[MAX_TARGETS];

    // Game stats
    int swingsPerformed = 0;
    int meleeHits = 0;
    int score = 0;

    // Initialize swings
    for (int i = 0; i < MAX_SWINGS; i++) {
        swings[i].active = false;
    }

    // Initialize targets at various positions
    Vector3 targetPositions[MAX_TARGETS] = {
        {-8.0f, 3.0f, -5.0f},   // Left front
        {8.0f, 3.0f, -5.0f},    // Right front
        {0.0f, 3.0f, -10.0f},   // Center back
        {-5.0f, 4.0f, 5.0f},    // Left back
        {5.0f, 4.0f, 5.0f}      // Right back
    };
    Color targetColors[MAX_TARGETS] = {RED, GREEN, BLUE, YELLOW, PURPLE};

    for (int i = 0; i < MAX_TARGETS; i++) {
        targets[i].position = targetPositions[i];
        targets[i].active = true;
        targets[i].hit = false;
        targets[i].color = targetColors[i];
    }

    // ===== COLLISION FUNCTIONS =====
    // Universal collision check using the environmental system
    auto checkUniversalCollision = [&](Vector3 position, float radius, float height, int excludeIndex = -1) -> bool {
        Vector3 boundsPos = position;
        boundsPos.y = playerY; // Use player's bottom position for collision
        CollisionBounds playerBounds = {
            CollisionShape::CAPSULE,
            boundsPos,
            {radius, height, 0.0f},
            0.0f
        };
        return environment.checkCollision(playerBounds, excludeIndex);
    };

    // NPC collision (still using old system for NPCs)
    auto capsuleNPCCollision = [&](Vector3 capsulePos, float capsuleRadius, float capsuleHeight, int excludeNPC = -1) -> bool {
        for (int n = 0; n < MAX_NPCS; n++) {
            if (n == excludeNPC) continue;

            // NPC cylinder: radius 0.6f, height 2.0f (slightly smaller for better interaction)
            const float npcRadius = 0.6f;
            const float npcHeight = 2.0f;

            // Check vertical overlap first
            float capsuleBottom = capsulePos.y;
            float capsuleTop = capsulePos.y + capsuleHeight;
            float npcBottom = npcs[n].position.y;
            float npcTop = npcs[n].position.y + npcHeight;

            if (capsuleTop < npcBottom || capsuleBottom > npcTop) {
                continue; // No vertical overlap
            }

            // Check horizontal distance (cylinder intersection)
            float dx = capsulePos.x - npcs[n].position.x;
            float dz = capsulePos.z - npcs[n].position.z;
            float distance2D = sqrtf(dx * dx + dz * dz);

            // Capsule-cylinder collision
            if (distance2D <= npcRadius + capsuleRadius) {
                return true; // Collision detected
            }
        }
        return false; // No collision
    };

    // Improved collision resolution with multi-directional wall sliding
    auto resolveCollisions = [&](Vector3& newPosition) {
        Vector3 originalPosition = camera.position;

        // ===== UNIVERSAL COLLISION DETECTION =====
        // Check collision with all environmental objects using universal system
        int excludeIndex = isInBuilding ? currentBuilding : -1;
        if (checkUniversalCollision(newPosition, PLAYER_RADIUS, PLAYER_HEIGHT, excludeIndex)) {
            // Multi-directional wall sliding for smoother movement
            Vector3 slidePosition = newPosition;
            Vector3 movement = {
                newPosition.x - originalPosition.x,
                newPosition.y - originalPosition.y,
                newPosition.z - originalPosition.z
            };

            const float WALL_SLIDE_THRESHOLD = 0.1f;

            // Try sliding in both X and Z directions simultaneously
            if (fabsf(movement.x) > WALL_SLIDE_THRESHOLD || fabsf(movement.z) > WALL_SLIDE_THRESHOLD) {
                // Try maintaining X movement while adjusting Z
                slidePosition.x = newPosition.x;
                slidePosition.z = originalPosition.z + (movement.z * 0.7f);
                if (!checkUniversalCollision(slidePosition, PLAYER_RADIUS, PLAYER_HEIGHT, excludeIndex)) {
                    newPosition = slidePosition;
                    return;
                }

                // Try maintaining Z movement while adjusting X
                slidePosition = newPosition;
                slidePosition.z = newPosition.z;
                slidePosition.x = originalPosition.x + (movement.x * 0.7f);
                if (!checkUniversalCollision(slidePosition, PLAYER_RADIUS, PLAYER_HEIGHT, excludeIndex)) {
                    newPosition = slidePosition;
                    return;
                }

                // Try diagonal sliding (both directions reduced)
                slidePosition = newPosition;
                slidePosition.x = originalPosition.x + (movement.x * 0.5f);
                slidePosition.z = originalPosition.z + (movement.z * 0.5f);
                if (!checkUniversalCollision(slidePosition, PLAYER_RADIUS, PLAYER_HEIGHT, excludeIndex)) {
                    newPosition = slidePosition;
                    return;
                }
            }

            // If no sliding is possible, try minimal movement in original direction
            slidePosition = originalPosition;
            slidePosition.x += movement.x * 0.3f;
            slidePosition.z += movement.z * 0.3f;
            if (!checkUniversalCollision(slidePosition, PLAYER_RADIUS, PLAYER_HEIGHT, excludeIndex)) {
                newPosition = slidePosition;
                return;
            }

            // Last resort: revert completely
            newPosition = originalPosition;
        }

        // Check NPC collisions with improved resolution
        if (capsuleNPCCollision(newPosition, PLAYER_RADIUS, PLAYER_HEIGHT)) {
            // Try to find a safe position by moving away from the NPC
            bool foundSafePosition = false;
            for (int attempt = 0; attempt < 8 && !foundSafePosition; attempt++) {
                float angle = (attempt * 45.0f) * DEG2RAD; // Try 8 directions
                float distance = PLAYER_RADIUS + 0.8f; // Move away by safe distance
                Vector3 testPosition = {
                    originalPosition.x + cosf(angle) * distance,
                    originalPosition.y,
                    originalPosition.z + sinf(angle) * distance
                };

                if (!capsuleNPCCollision(testPosition, PLAYER_RADIUS, PLAYER_HEIGHT)) {
                    newPosition = testPosition;
                    foundSafePosition = true;
                }
            }

            // If no safe position found, revert to original (last resort)
            if (!foundSafePosition) {
                newPosition = originalPosition;
            }
        }

        // If inside building, clamp position to interior bounds to prevent walking through walls
        if (isInBuilding) {
            auto objects = environment.getAllObjects();
            if (currentBuilding >= 0 && currentBuilding < static_cast<int>(objects.size())) {
                if (auto building = std::dynamic_pointer_cast<Building>(objects[currentBuilding])) {
                    Vector3 bSize = building->getSize();
                    Vector3 bPos = building->position;

                    float halfW = bSize.x / 2.0f - PLAYER_RADIUS;
                    float halfH = bSize.y / 2.0f - 0.1f; // Slight headroom
                    float halfD = bSize.z / 2.0f - PLAYER_RADIUS;

                    float minX = bPos.x - halfW;
                    float maxX = bPos.x + halfW;
                    float minY = groundLevel;
                    float maxY = bPos.y + halfH;
                    float minZ = bPos.z - halfD;
                    float maxZ = bPos.z + halfD;

                    newPosition.x = std::max(minX, std::min(maxX, newPosition.x));
                    newPosition.y = std::max(minY + eyeHeight, std::min(maxY, newPosition.y));
                    newPosition.z = std::max(minZ, std::min(maxZ, newPosition.z));
                }
            }
        }
    };

    // ===== STANDARDIZED BUILDING SYSTEM =====
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

    // Initialize NPCs for the two buildings (GROUND LEVEL positions)
    // Mayor's Building: Center at {0.0f, 2.5f, -12.0f}, Size {10.0f, 5.0f, 8.0f}
    // Player enters at {0.0f, 1.75f, -10.0f} (center.z + 2.0f)
    npcs[0] = {
        .position = {0.0f, 0.0f, -15.5f},   // FARTHER back in Mayor's Building (was -14.0f)
        .name = "Mayor White",
        .dialog = "Greetings, citizen! Welcome to my office. What brings you here today?",
        .color = WHITE,
        .canInteract = true,
        .interactionRadius = 2.5f                // Reduced radius (was 3.0f)
    };

    // Shop Keeper's Building: Center at {12.0f, 2.5f, 0.0f}, Size {8.0f, 5.0f, 6.0f}
    // Player enters at {12.0f, 1.75f, 2.0f} (center.z + 2.0f)
    npcs[1] = {
        .position = {12.0f, 0.0f, -5.0f},    // FARTHER back in Shop Building (was -3.0f)
        .name = "Buster Shoppin",
        .dialog = "Welcome to my shop! I've got all sorts of goods and supplies for sale.",
        .color = GREEN,
        .canInteract = true,
        .interactionRadius = 2.5f                // Reduced radius (was 3.0f)
    };

    // ===== ENVIRONMENTAL OBJECTS =====
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



    // Real-time testing variables
    bool testMouseCaptured = false;
    bool testBuildingCollision = false; // Test variable for collision detection
    bool testWASDMovement = false;
    bool testSpaceJump = false;
    bool testMouseLook = false;
    bool testMeleeSwing = false;
    bool testMeleeHitDetection = false;
    // testBuildingEntry is now in GameState structure
    bool testNPCInteraction = false;
    Vector3 lastCameraPos = camera.position;

    // Main game loop with custom ESC handling
    float lastEscPressTime = -10.0f; // Initialize to a time long ago
    bool shouldClose = false;
    while (!shouldClose)  // Custom close detection
    {
        float currentTime = GetTime();

        // Handle ESC key for mouse capture/release and dialog exit
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (currentTime - lastEscPressTime < 0.5f) {
                // Double ESC press within 0.5 seconds: Close window
                shouldClose = true;
                break;
            } else {
                // If in dialog mode, exit dialog first
                if (isInDialog) {
                    isInDialog = false;
                    showDialogWindow = false;
                    // Re-capture mouse cursor for FPS gameplay (only if not released by user)
                    if (!mouseReleased) {
                        DisableCursor();
                    }
                } else {
                    // Single ESC press: Toggle mouse capture
                    if (!mouseReleased) {
                        // First ESC press: Release mouse cursor
                        EnableCursor();
                        mouseReleased = true;
                    } else {
                        // ESC while mouse is released: Re-capture mouse
                        DisableCursor();
                        mouseReleased = false;
                    }
                }
                lastEscPressTime = currentTime;
            }
        }

        // Check for window close button (X button or Alt+F4)
        if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4)) {
            shouldClose = true;
            break;
        }

        // Only check WindowShouldClose() if ESC wasn't pressed recently
        if (WindowShouldClose() && (currentTime - lastEscPressTime) > 1.0f) {
            shouldClose = true;
            break;
        }

        // Real-time testing: Check mouse capture
        if ((IsCursorHidden() && !mouseReleased) || isInDialog) {  // Mouse should be captured during dialog too
            testMouseCaptured = true;
        }

        // Real-time testing: Check WASD movement
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)) {
            testWASDMovement = true;
        }

        // Real-time testing: Check spacebar jump
        if (IsKeyPressed(KEY_SPACE)) {
            testSpaceJump = true;
        }

        // Real-time testing: Check mouse movement (look)
        if (GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f) {
            testMouseLook = true;
        }

        // Real-time testing: Check melee swing
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            testMeleeSwing = true;
        }

        // ===== INTERACTION SYSTEM (E key) =====
        // Priority system: Check NPC interactions first when inside building, then door interactions

        static float lastEPressTime = 0.0f;
        bool eKeyPressed = IsKeyPressed(KEY_E) && (currentTime - lastEPressTime) > 0.3f;

        if (eKeyPressed) {
            lastEPressTime = currentTime;
        }

        // Flag to prevent multiple interactions in same frame
        bool interactionHandled = false;

        // ===== NPC INTERACTIONS (Higher Priority when inside building) =====
        if (isInBuilding && !interactionHandled) {
            for (int n = 0; n < MAX_NPCS; n++) {
                bool npcVisible = false;

                // Determine if NPC should be interactable based on current location
                if (isInBuilding) {
                    // Inside building - only interact with NPCs in current building
                    if (currentBuilding == 0 && n == 0) npcVisible = true; // Mayor White in Mayor's Building
                    if (currentBuilding == 1 && n == 1) npcVisible = true; // Buster Shoppin in Shop Building
                }

                if (npcVisible) {
                    Vector3 toNPC = {
                        npcs[n].position.x - camera.position.x,
                        npcs[n].position.y - camera.position.y,
                        npcs[n].position.z - camera.position.z
                    };
                    float distance = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y + toNPC.z * toNPC.z);

                    // Increased interaction range for better usability
                    if (distance <= npcs[n].interactionRadius * 1.5f) {
                        showInteractPrompt = true;
                        interactPromptText = "Press E to talk to " + npcs[n].name;
                        if (eKeyPressed && !isInDialog) {
                            // Start dialog with NPC - HIGHEST PRIORITY
                            startDialog(n);
                            testNPCInteraction = true;
                            interactionHandled = true;  // Prevent door interaction
                            break;  // Exit NPC loop once interaction is handled
                        }
                    }
                }
            }
        }

        // ===== BUILDING INTERACTIONS (Lower Priority) =====
        if (!interactionHandled) {
            auto objects = environment.getAllObjects();
            for (size_t i = 0; i < objects.size(); ++i) {
                if (auto building = std::dynamic_pointer_cast<Building>(objects[i])) {
                    if (building->isInteractive()) {
                        Vector3 doorPos = building->getDoorPosition();
                        Vector3 toDoor = {
                            doorPos.x - camera.position.x,
                            doorPos.y - camera.position.y,
                            doorPos.z - camera.position.z
                        };
                        float distance = sqrtf(toDoor.x * toDoor.x + toDoor.y * toDoor.y + toDoor.z * toDoor.z);

                        if (distance <= 3.0f && !isInBuilding) {
                            showInteractPrompt = true;
                            interactPromptText = "Press E to enter " + building->getName();
                            if (eKeyPressed) {
                                // Enter building - position player safely inside, away from walls and NPCs
                                isInBuilding = true;
                                currentBuilding = static_cast<int>(i);
                                lastOutdoorPosition = camera.position;

                                // Position player 1 unit inside from the door (safer positioning)
                                camera.position = {building->position.x, eyeHeight, building->position.z + 1.0f};
                                camera.target = {building->position.x, eyeHeight - 0.2f, building->position.z + 4.0f};
                                playerY = 0.0f;
                                testBuildingEntry = true;
                                interactionHandled = true;
                                break; // Prevent multiple building entries
                            }
                        } else if (isInBuilding && currentBuilding == static_cast<int>(i) && eKeyPressed) {
                            // Exit building (only current building) - LOWEST PRIORITY
                            isInBuilding = false;
                            camera.position = lastOutdoorPosition;
                            camera.target = {lastOutdoorPosition.x, lastOutdoorPosition.y - 0.2f, lastOutdoorPosition.z - 5.0f};
                            currentBuilding = -1;
                            interactionHandled = true;
                            break; // Prevent multiple exits
                        }
                    }
                }
            }
        }

        // Handle dialog option selection with improved mouse clicks
        if (isInDialog && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            // Use the same centered dialog positioning as rendering
            int dialogWidth = 600;
            int dialogHeight = 200;
            int dialogX = (screenWidth - dialogWidth) / 2;
            int dialogY = (screenHeight - dialogHeight) / 2;

            // Button positioning relative to centered dialog (same as rendering)
            int buttonY = dialogY + 100; // Same as rendering
            const int buttonHeight = 30;
            const int buttonWidth = 150;
            const int buttonSpacing = 170;
            int startX = dialogX + 25; // Same as rendering

            for (int i = 0; i < numDialogOptions; i++) {
                int buttonX = startX + (i * buttonSpacing);
                int buttonRight = buttonX + buttonWidth;
                int buttonBottom = buttonY + buttonHeight;

                // More generous hit area with some padding
                int hitPadding = 5;
                if (mousePos.x >= buttonX - hitPadding && mousePos.x <= buttonRight + hitPadding &&
                    mousePos.y >= buttonY - hitPadding && mousePos.y <= buttonBottom + hitPadding) {
                    handleDialogOption(i);
                    break; // Only handle one button click per frame
                }
            }
        }

        // ===== INTERACTION PROMPT SYSTEM =====
        static float lastInteractionTime = 0.0f;
        static bool wasNearInteractable = false;

        if (showInteractPrompt) {
            lastInteractionTime = currentTime;
            wasNearInteractable = true;
        } else {
            // Improved prompt fading - only clear after moving away for a short time
            if (wasNearInteractable && (currentTime - lastInteractionTime) > 0.5f) {
                showInteractPrompt = false;
                wasNearInteractable = false;
            }
            // If we press E while prompt is visible, keep it visible briefly
            if (eKeyPressed && showInteractPrompt) {
                lastInteractionTime = currentTime;
            }
        }

        // Real-time testing: Check building entry
        if (isInBuilding) {
            testBuildingEntry = true;
        }

        // Real-time testing: Check NPC interaction
        if (showInteractPrompt && interactPromptText.find("talk to") != std::string::npos) {
            testNPCInteraction = true;
        }

        // Melee combat mechanics (longsword) - disabled during dialog
        if (!isInDialog && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (currentTime - lastSwingTime) > swingCooldown) {
            // Find inactive swing
            for (int i = 0; i < MAX_SWINGS; i++) {
                if (!swings[i].active) {
                    // Create new swing
                    swings[i].startPosition = camera.position;
                    swings[i].startPosition.y -= 0.5f; // Adjust for hand level

                    // Calculate direction based on camera forward vector (proper aiming)
                    Vector3 forward = {
                        camera.target.x - camera.position.x,
                        camera.target.y - camera.position.y,
                        camera.target.z - camera.position.z
                    };

                    // Normalize the direction vector
                    float length = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
                    if (length > 0) {
                        forward.x /= length;
                        forward.y /= length;
                        forward.z /= length;
                    }

                    swings[i].direction = forward;
                    swings[i].endPosition.x = swings[i].startPosition.x + forward.x * swingRange;
                    swings[i].endPosition.y = swings[i].startPosition.y + forward.y * swingRange;
                    swings[i].endPosition.z = swings[i].startPosition.z + forward.z * swingRange;

                    swings[i].active = true;
                    swings[i].progress = 0.0f;
                    swings[i].lifetime = swingDuration;
                    lastSwingTime = currentTime;
                    swingsPerformed++;
                    testMeleeSwing = true;

                    // Check for immediate hits (melee is instant)
                    for (int t = 0; t < MAX_TARGETS; t++) {
                        if (targets[t].active && !targets[t].hit) {
                            Vector3 toTarget = {
                                targets[t].position.x - swings[i].startPosition.x,
                                targets[t].position.y - swings[i].startPosition.y,
                                targets[t].position.z - swings[i].startPosition.z
                            };
                            float distance = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);

                            if (distance <= swingRange) {
                                // Hit detected!
                                targets[t].hit = true;
                                targets[t].hitTime = GetTime();
                                meleeHits++;
                                score += 150; // Higher score for melee hits
                                testMeleeHitDetection = true;
                            }
                        }
                    }
                    break;
                }
            }
        }

        // Update swings
        for (int i = 0; i < MAX_SWINGS; i++) {
            if (swings[i].active) {
                swings[i].progress += swingSpeed * GetFrameTime();
                swings[i].lifetime -= GetFrameTime();

                // Deactivate if swing completed
                if (swings[i].progress >= 1.0f || swings[i].lifetime <= 0) {
                    swings[i].active = false;
                }
            }
        }

        // Melee hit detection is handled immediately during swing creation

        // Update targets (respawn after being hit)
        for (int t = 0; t < MAX_TARGETS; t++) {
            if (targets[t].hit && (GetTime() - targets[t].hitTime) > 2.0f) {
                // Respawn target
                targets[t].hit = false;
                targets[t].active = true;
            }
        }

        // Update camera position tracking
        lastCameraPos = camera.position;

        // Custom jumping system (works with raylib's FPS camera) - disabled during dialog
        if (!isInDialog && IsKeyPressed(KEY_SPACE) && isGrounded && !isJumping) {
            // Start jump
            isJumping = true;
            isGrounded = false;
            jumpVelocity = jumpStrength;
            testSpaceJump = true; // Mark jump test as passed
        }

        // Apply gravity and update vertical position
        if (isJumping || !isGrounded) {
            jumpVelocity += gravity * GetFrameTime(); // Apply gravity
            playerY += jumpVelocity * GetFrameTime(); // Update player bottom Y

            // Check if landed on ground
            if (playerY <= groundLevel) {
                playerY = groundLevel;
                isJumping = false;
                isGrounded = true;
                jumpVelocity = 0.0f;
            }
        }

        camera.position.y = playerY + eyeHeight;
        // Removed: camera.target.y = camera.position.y - 0.2f;  // This was forcing vertical look reset

        // Update
        // Only update camera when not in dialog mode
        if (!isInDialog) {
            // Store original position before any movement
            Vector3 originalPosition = camera.position;

            // Calculate intended new position from camera controls
            UpdateCamera(&camera, CAMERA_FIRST_PERSON);  // Update camera (movement and looking)
            Vector3 intendedPosition = camera.position;

            // Reset camera to original position for collision checking
            camera.position = originalPosition;

            // Apply collision detection and resolution to intended position
            resolveCollisions(intendedPosition);
            camera.position = intendedPosition;

            // Update playerY based on new camera y
            playerY = camera.position.y - eyeHeight;
        }

        // Real-time testing: Check if camera actually moved
        if (camera.position.x != lastCameraPos.x ||
            camera.position.y != lastCameraPos.y ||
            camera.position.z != lastCameraPos.z) {
            testWASDMovement = true; // Movement detected
        }

        // Check for collision feedback
        testBuildingCollision = false; // Reset each frame
        Vector3 testPosition = camera.position;
        testPosition.x += 0.1f; // Test forward movement
        int exclude = isInBuilding ? currentBuilding : -1;
        if (checkUniversalCollision(testPosition, PLAYER_RADIUS, PLAYER_HEIGHT, exclude)) {
            testBuildingCollision = true; // Collision detected
        }

        // Draw
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw ground plane with dynamic color based on jump state
                Color groundColor = LIGHTGRAY;
                if (isJumping) {
                    groundColor = Fade(SKYBLUE, 0.8f); // Blue tint when jumping
                } else if (!isGrounded) {
                    groundColor = Fade(YELLOW, 0.6f); // Yellow when falling
                }
                DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 16.0f, 16.0f }, groundColor);

                // ===== BUILDING INTERIOR RENDERING =====
                // Render building interiors FIRST (before environment) when player is inside
                if (isInBuilding && currentBuilding >= 0) {
                    auto objects = environment.getAllObjects();
                    if (currentBuilding < static_cast<int>(objects.size())) {
                        if (auto building = std::dynamic_pointer_cast<Building>(objects[currentBuilding])) {
                            // Draw solid interior walls - individual faces for proper enclosure
                            Vector3 buildingSize = building->getSize();
                            Vector3 pos = building->position;
                            Color interiorWallColor = building->getColor();

                            // Make interior walls slightly smaller to avoid z-fighting with exterior
                            float wallThickness = 0.3f; // Thick walls for proper enclosure
                            float interiorInset = 0.1f;  // Small inset from exterior walls

                            // Calculate interior dimensions (smaller than exterior)
                            float interiorWidth = buildingSize.x - (2 * interiorInset);
                            float interiorHeight = buildingSize.y - (2 * interiorInset);
                            float interiorDepth = buildingSize.z - (2 * interiorInset);

                            // ===== DRAW INDIVIDUAL WALL FACES WITH DOOR OPENING =====

                            // Get door position to create opening in the correct wall
                            Vector3 doorPos = building->getDoorPosition();
                            Vector3 doorOffset = {
                                doorPos.x - building->position.x,
                                doorPos.y - building->position.y,
                                doorPos.z - building->position.z
                            };

                            // Door dimensions for creating wall opening
                            float doorWidth = 1.2f;
                            float doorHeight = 2.8f;

                            // Front wall (North face - +Z) - check if door is on this face
                            if (fabs(doorOffset.z - (buildingSize.z/2)) > 0.5f) { // Door not on this face
                                Vector3 frontWallPos = {pos.x, pos.y, pos.z + interiorDepth/2};
                                DrawCube(frontWallPos, interiorWidth, interiorHeight, wallThickness, Fade(interiorWallColor, 1.0f));
                            } else { // Door is on this face - create opening
                                // Left part of front wall
                                float leftWallWidth = (interiorWidth/2) + doorOffset.x - (doorWidth/2);
                                if (leftWallWidth > 0.1f) {
                                    Vector3 leftWallPos = {pos.x - (interiorWidth/2 - leftWallWidth/2), pos.y, pos.z + interiorDepth/2};
                                    DrawCube(leftWallPos, leftWallWidth, interiorHeight, wallThickness, Fade(interiorWallColor, 1.0f));
                                }
                                // Right part of front wall
                                float rightWallWidth = (interiorWidth/2) - doorOffset.x - (doorWidth/2);
                                if (rightWallWidth > 0.1f) {
                                    Vector3 rightWallPos = {pos.x + (interiorWidth/2 - rightWallWidth/2), pos.y, pos.z + interiorDepth/2};
                                    DrawCube(rightWallPos, rightWallWidth, interiorHeight, wallThickness, Fade(interiorWallColor, 1.0f));
                                }
                                // Top part of front wall (above door)
                                Vector3 topWallPos = {pos.x, pos.y + (interiorHeight/2 - doorHeight/2), pos.z + interiorDepth/2};
                                DrawCube(topWallPos, doorWidth + 0.2f, interiorHeight - doorHeight, wallThickness, Fade(interiorWallColor, 1.0f));
                            }

                            // Back wall (South face - -Z)
                            Vector3 backWallPos = {pos.x, pos.y, pos.z - interiorDepth/2};
                            DrawCube(backWallPos, interiorWidth, interiorHeight, wallThickness, Fade(interiorWallColor, 1.0f));

                            // Left wall (West face - -X)
                            Vector3 leftWallPos = {pos.x - interiorWidth/2, pos.y, pos.z};
                            DrawCube(leftWallPos, wallThickness, interiorHeight, interiorDepth, Fade(interiorWallColor, 1.0f));

                            // Right wall (East face - +X)
                            Vector3 rightWallPos = {pos.x + interiorWidth/2, pos.y, pos.z};
                            DrawCube(rightWallPos, wallThickness, interiorHeight, interiorDepth, Fade(interiorWallColor, 1.0f));

                            // Draw interior floor at consistent world ground level (Y=0)
                            Vector3 floorPos = {building->position.x, 0.0f, building->position.z};
                            DrawCube(floorPos, interiorWidth, 0.1f, interiorDepth, DARKGRAY);

                            // Draw interior ceiling at the top of the building (not halfway up!)
                            Vector3 ceilingPos = {building->position.x, building->position.y + buildingSize.y/2 - 0.05f, building->position.z};
                            DrawCube(ceilingPos, interiorWidth, 0.1f, interiorDepth, LIGHTGRAY);

                            // Interior walls with door opening complete
                        }
                    }
                }

                // ===== RENDER ENVIRONMENTAL OBJECTS =====
                environment.renderAll();

                // ===== INTERACTION SYSTEM =====
                // Handle building interactions
                auto interactiveObjects = environment.getInteractiveObjects();
                for (const auto& obj : interactiveObjects) {
                    if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                        Vector3 doorPos = building->getDoorPosition();
                        Vector3 toDoor = {
                            doorPos.x - camera.position.x,
                            doorPos.y - camera.position.y,
                            doorPos.z - camera.position.z
                        };
                        float distance = sqrtf(toDoor.x * toDoor.x + toDoor.y * toDoor.y + toDoor.z * toDoor.z);

                        if (distance <= 3.0f && !isInBuilding) {
                            // Bright, pulsing interaction indicator
                            float pulse = 0.7f + sinf(currentTime * 5.0f) * 0.3f;
                            Vector3 indicatorPos = {doorPos.x, doorPos.y + 3.0f, doorPos.z};
                            DrawSphere(indicatorPos, 0.25f * pulse, YELLOW);

                            // Enhanced interaction range visualization
                            DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.3f));
                        } else if (distance <= 5.0f && !isInBuilding) {
                            // Subtle approach indicator
                            DrawCircle3D(doorPos, 3.0f, {0, 1, 0}, 90, Fade(YELLOW, 0.1f));
                        }
                    }
                }

                // Draw NPCs (only show NPCs that are in the current area)
                for (int n = 0; n < MAX_NPCS; n++) {
                    bool shouldDrawNPC = false;

                    // Determine if NPC should be visible based on current location
                    if (isInBuilding) {
                        // Inside building - only show NPCs that are inside the current building
                        if (currentBuilding == 0 && n == 0) shouldDrawNPC = true; // Mayor White in Mayor's Building
                        if (currentBuilding == 1 && n == 1) shouldDrawNPC = true; // Buster Shoppin in Shop Building
                    } else {
                        // Outside - NPCs are not visible from outside (they're inside buildings)
                        shouldDrawNPC = false;
                    }

                    if (shouldDrawNPC) {
                        // ===== ENHANCED HUMANOID NPC RENDERING =====
                        // NPC positions are FIXED and INDEPENDENT of door positions

                        // NPC positions are now properly set at ground level

                        // Draw NPC body (more detailed humanoid shape)
                        DrawCylinder(npcs[n].position, 0.8f, 0.5f, 1.6f, 12, npcs[n].color); // Taller, more detailed body
                        DrawCylinderWires(npcs[n].position, 0.8f, 0.5f, 1.6f, 12, BLACK);

                        // Draw NPC head (more detailed and visible)
                        Vector3 headPos = {npcs[n].position.x, npcs[n].position.y + 1.8f, npcs[n].position.z};
                        DrawSphere(headPos, 0.45f, npcs[n].color); // Larger, more visible head
                        DrawSphereWires(headPos, 0.45f, 12, 8, BLACK);

                        // Draw NPC arms (make them more humanoid)
                        Vector3 leftArmPos = {npcs[n].position.x - 0.6f, npcs[n].position.y + 0.8f, npcs[n].position.z};
                        Vector3 rightArmPos = {npcs[n].position.x + 0.6f, npcs[n].position.y + 0.8f, npcs[n].position.z};
                        DrawCylinder(leftArmPos, 0.15f, 0.15f, 0.8f, 8, Fade(npcs[n].color, 0.9f));
                        DrawCylinder(rightArmPos, 0.15f, 0.15f, 0.8f, 8, Fade(npcs[n].color, 0.9f));

                        // Draw NPC legs (more detailed)
                        Vector3 leftLegPos = {npcs[n].position.x - 0.25f, npcs[n].position.y - 0.4f, npcs[n].position.z};
                        Vector3 rightLegPos = {npcs[n].position.x + 0.25f, npcs[n].position.y - 0.4f, npcs[n].position.z};
                        DrawCylinder(leftLegPos, 0.2f, 0.15f, 0.8f, 8, Fade(npcs[n].color, 0.8f));
                        DrawCylinder(rightLegPos, 0.2f, 0.15f, 0.8f, 8, Fade(npcs[n].color, 0.8f));

                        // Add subtle glow effect around NPC for better visibility
                        DrawSphere(npcs[n].position, 1.2f, Fade(npcs[n].color, 0.1f));

                        // ===== ENHANCED INTERACTION SYSTEM =====
                        Vector3 toNPC = {
                            npcs[n].position.x - camera.position.x,
                            npcs[n].position.y - camera.position.y,
                            npcs[n].position.z - camera.position.z
                        };
                        float distance = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y + toNPC.z * toNPC.z);

                        if (distance <= npcs[n].interactionRadius) {
                            // Bright, pulsing interaction indicator
                            float pulse = 0.8f + sinf(currentTime * 6.0f) * 0.4f;
                            Vector3 indicatorPos = {npcs[n].position.x, npcs[n].position.y + 3.0f, npcs[n].position.z};
                            DrawSphere(indicatorPos, 0.2f * pulse, GREEN);

                            // Enhanced interaction range (more visible)
                            DrawCircle3D(npcs[n].position, npcs[n].interactionRadius, {0, 1, 0}, 90, Fade(GREEN, 0.4f));

                            // Highlight NPC when in range (mouseover effect)
                            DrawCylinder(npcs[n].position, 0.85f, 0.55f, 1.65f, 12, Fade(YELLOW, 0.2f));
                        } else if (distance <= npcs[n].interactionRadius * 1.8f) {
                            // Approach indicator (longer range)
                            DrawCircle3D(npcs[n].position, npcs[n].interactionRadius, {0, 1, 0}, 90, Fade(YELLOW, 0.15f));
                        }
                    }
                }

                // ===== ENVIRONMENTAL OBJECTS RENDERED =====
                // All environmental objects (well, trees, buildings) are now rendered through the universal environmental system above

                // Location indicator text removed for clean UI

                // Draw longsword swings
                for (int s = 0; s < MAX_SWINGS; s++) {
                    if (swings[s].active) {
                        // Calculate current swing position based on progress
                        Vector3 currentPos = {
                            swings[s].startPosition.x + swings[s].direction.x * swingRange * swings[s].progress,
                            swings[s].startPosition.y + swings[s].direction.y * swingRange * swings[s].progress,
                            swings[s].startPosition.z + swings[s].direction.z * swingRange * swings[s].progress
                        };

                        // Draw the sword blade
                        DrawCylinderEx(swings[s].startPosition, currentPos, 0.02f, 0.01f, 8, RED);

                        // Draw sword handle
                        Vector3 handleStart = swings[s].startPosition;
                        Vector3 handleEnd = {
                            handleStart.x + swings[s].direction.x * 0.5f,
                            handleStart.y + swings[s].direction.y * 0.5f,
                            handleStart.z + swings[s].direction.z * 0.5f
                        };
                        DrawCylinderEx(handleStart, handleEnd, 0.03f, 0.02f, 8, DARKBROWN);

                        // Draw sword glow effect
                        DrawSphere(currentPos, 0.1f, Fade(YELLOW, 0.7f));

                        // Draw swing arc trail
                        for (float trail = 0.2f; trail <= swings[s].progress; trail += 0.2f) {
                            Vector3 trailPos = {
                                swings[s].startPosition.x + swings[s].direction.x * swingRange * trail,
                                swings[s].startPosition.y + swings[s].direction.y * swingRange * trail,
                                swings[s].startPosition.z + swings[s].direction.z * swingRange * trail
                            };
                            DrawSphere(trailPos, 0.03f, Fade(ORANGE, 0.3f * (1.0f - trail)));
                        }
                    }
                }

                // Draw melee range indicator when attacking
                bool isSwinging = false;
                for (int s = 0; s < MAX_SWINGS; s++) {
                    if (swings[s].active) {
                        isSwinging = true;
                        break;
                    }
                }

                if (isSwinging) {
                    // Draw melee range sphere (rings=8, slices=16 for good detail)
                    DrawSphereWires(camera.position, swingRange, 8, 16, Fade(RED, 0.5f));
                }

                // Draw jump height indicator (optional visual feedback)
                if (isJumping) {
                    // Debug jump height text removed for clean UI
                }

            EndMode3D();

            // Draw crosshair/aiming reticle (only when not in dialog and mouse is captured)
            if (!isInDialog && !mouseReleased) {
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
            DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
            } else if (isInDialog) {
                // Draw dialog mode indicator
                int centerX = screenWidth / 2;
                int centerY = screenHeight / 2;
                DrawCircle(centerX, centerY, 8, Fade(WHITE, 0.5f));
                DrawCircleLines(centerX, centerY, 8, WHITE);
                DrawText("DIALOG", centerX - 25, centerY - 25, 10, YELLOW);
            } else if (mouseReleased) {
                // Draw mouse released indicator with ESC instructions
                int centerX = screenWidth / 2;
                int centerY = screenHeight / 2;
                DrawRectangle(centerX - 60, centerY - 20, 120, 40, Fade(BLACK, 0.8f));
                DrawRectangleLines(centerX - 60, centerY - 20, 120, 40, YELLOW);
                DrawText("MOUSE FREE", centerX - 45, centerY - 15, 12, WHITE);
                DrawText("ESC: RECAPTURE", centerX - 55, centerY + 2, 10, YELLOW);
            }

            // Optional: Draw FPS for debugging
            DrawFPS(10, 10);

            // Persistent testing display (always visible, no blinking)

            // ===== IMPROVED UI LAYOUT SYSTEM =====
            // Organized layout to prevent overlaps and improve readability

            // Left Panel - Status & Controls
            int leftPanelX = 10;
            int leftPanelY = 25;
            int leftPanelWidth = 320;
            int leftPanelHeight = 350;

            DrawRectangle(leftPanelX, leftPanelY, leftPanelWidth, leftPanelHeight, Fade(BLACK, 0.85f));
            DrawRectangleLines(leftPanelX, leftPanelY, leftPanelWidth, leftPanelHeight, BLUE);

            // Title
            DrawText("🎮 BROWSERWIND - STATUS", leftPanelX + 10, leftPanelY + 5, 16, SKYBLUE);
            int statusY = leftPanelY + 25;

            // Movement & Controls Section
            DrawText("─── MOVEMENT & CONTROLS ───", leftPanelX + 10, statusY, 12, YELLOW);
            statusY += 16;

            // Mouse capture test
            Color mouseColor = testMouseCaptured ? GREEN : RED;
            DrawText(TextFormat("🖱️  Mouse: %s", testMouseCaptured ? "✓ WORKING" : "✗ BROKEN"), leftPanelX + 15, statusY, 10, mouseColor);
            statusY += 14;

            // WASD movement test
            Color wasdColor = testWASDMovement ? GREEN : ORANGE;
            DrawText(TextFormat("🏃 WASD: %s", testWASDMovement ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, wasdColor);
            statusY += 14;

            // Space jump test
            Color jumpColor = testSpaceJump ? GREEN : ORANGE;
            DrawText(TextFormat("🦘 Space: %s", testSpaceJump ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, jumpColor);
            statusY += 14;

            // Mouse look test
            Color lookColor = testMouseLook ? GREEN : ORANGE;
            DrawText(TextFormat("👁️  Look: %s", testMouseLook ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, lookColor);
            statusY += 16;

            // Combat Section
            DrawText("─── COMBAT ───", leftPanelX + 10, statusY, 12, YELLOW);
            statusY += 16;

            // Melee swing test
            Color swingColor = testMeleeSwing ? GREEN : ORANGE;
            DrawText(TextFormat("⚔️  Attack: %s", testMeleeSwing ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, swingColor);
            statusY += 14;

            // Melee hit detection test
            Color hitColor = testMeleeHitDetection ? GREEN : ORANGE;
            DrawText(TextFormat("🎯 Hits: %s", testMeleeHitDetection ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, hitColor);
            statusY += 16;

            // World Interaction Section
            DrawText("─── WORLD ───", leftPanelX + 10, statusY, 12, YELLOW);
            statusY += 16;

            // Building entry test
            Color buildingColor = testBuildingEntry ? GREEN : ORANGE;
            DrawText(TextFormat("🏛️  Buildings: %s", testBuildingEntry ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, buildingColor);
            statusY += 14;

            // NPC interaction test
            Color npcColor = testNPCInteraction ? GREEN : ORANGE;
            DrawText(TextFormat("👥 NPCs: %s", testNPCInteraction ? "✓ TESTED" : "⏳ UNTESTED"), leftPanelX + 15, statusY, 10, npcColor);
            statusY += 16;

            // Status Section
            DrawText("─── STATUS ───", leftPanelX + 10, statusY, 12, YELLOW);
            statusY += 16;

            // Mouse state
            Color mouseStateColor = mouseReleased ? ORANGE : GREEN;
            DrawText(TextFormat("🖱️  %s", mouseReleased ? "FREE" : "CAPTURED"), leftPanelX + 15, statusY, 10, mouseStateColor);
            statusY += 14;

            // Current location
            Color locationColor = isInBuilding ? BLUE : GREEN;
            std::string locationText = "Town Square";
            if (isInBuilding && currentBuilding >= 0) {
                auto objects = environment.getAllObjects();
                if (currentBuilding < static_cast<int>(objects.size())) {
                    locationText = objects[currentBuilding]->getName();
                }
            }
            DrawText(TextFormat("📍 %s", locationText.c_str()), leftPanelX + 15, statusY, 10, locationColor);
            statusY += 14;

            // Dialog state
            Color dialogColor = isInDialog ? PURPLE : GRAY;
            DrawText(TextFormat("💬 %s", isInDialog ? "ACTIVE" : "INACTIVE"), leftPanelX + 15, statusY, 10, dialogColor);
            statusY += 16;

            // Instructions (compact)
            DrawText("🎯 TEST: WASD+Mouse+Space+LMB+E", leftPanelX + 10, statusY, 9, WHITE);
            statusY += 12;
            DrawText("ESC: Toggle mouse capture", leftPanelX + 10, statusY, 9, LIGHTGRAY);

            // ===== BOTTOM LEFT PANEL - GAME STATISTICS =====
            int statsPanelY = leftPanelY + leftPanelHeight + 10;
            int statsPanelHeight = 120;

            DrawRectangle(leftPanelX, statsPanelY, leftPanelWidth, statsPanelHeight, Fade(BLACK, 0.85f));
            DrawRectangleLines(leftPanelX, statsPanelY, leftPanelWidth, statsPanelHeight, GREEN);

            DrawText("📊 GAME STATISTICS", leftPanelX + 10, statsPanelY + 5, 14, LIME);
            DrawText(TextFormat("🏆 Score: %d points", score), leftPanelX + 15, statsPanelY + 25, 11, WHITE);
            DrawText(TextFormat("⚔️  Swings: %d", swingsPerformed), leftPanelX + 15, statsPanelY + 38, 11, WHITE);
            DrawText(TextFormat("🎯 Hits: %d", meleeHits), leftPanelX + 15, statsPanelY + 51, 11, WHITE);
            float accuracy = swingsPerformed > 0 ? (float)meleeHits / swingsPerformed * 100.0f : 0.0f;
            DrawText(TextFormat("📈 Accuracy: %.1f%%", accuracy), leftPanelX + 15, statsPanelY + 64, 11, WHITE);
            DrawText(TextFormat("🏢 Buildings: %d/2", testBuildingEntry ? 1 : 0), leftPanelX + 15, statsPanelY + 77, 11, BLUE);
            DrawText("🟢 WORKING | 🟠 UNTESTED | 🔴 BROKEN", leftPanelX + 10, statsPanelY + 95, 9, GRAY);

            // ===== TOP-RIGHT INTERACTION PROMPT =====
            if (showInteractPrompt && !isInDialog) {
                int promptX = screenWidth - 320;
                int promptY = 100;
                int promptWidth = 300;
                int promptHeight = 50;

                // Animated background for better visibility
                float alpha = 0.8f + sinf(currentTime * 3.0f) * 0.1f; // Subtle pulsing
                DrawRectangle(promptX, promptY, promptWidth, promptHeight, Fade(BLACK, alpha));
                DrawRectangleLines(promptX, promptY, promptWidth, promptHeight, YELLOW);

                // Draw prompt text with better formatting
                DrawText(interactPromptText.c_str(), promptX + 10, promptY + 8, 14, YELLOW);
                DrawText("[E] to interact", promptX + 10, promptY + 25, 12, WHITE);

                // Add a small icon indicator
                DrawCircle(promptX + promptWidth - 15, promptY + promptHeight - 15, 3, YELLOW);
            }

            // Dialog window
            if (isInDialog && showDialogWindow) {
                // Center the dialog window on screen to avoid overlap with top left UI
                int dialogWidth = 600;
                int dialogHeight = 200;
                int dialogX = (screenWidth - dialogWidth) / 2;
                int dialogY = (screenHeight - dialogHeight) / 2;

                // Main dialog background
                DrawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, Fade(BLACK, 0.9f));
                DrawRectangleLines(dialogX, dialogY, dialogWidth, dialogHeight, WHITE);

                // Dialog title (centered)
                int titleX = dialogX + (dialogWidth - MeasureText("Conversation", 18)) / 2;
                DrawText("Conversation", titleX, dialogY + 15, 18, SKYBLUE);

                // Dialog text (centered)
                int textX = dialogX + 20;
                DrawText(dialogText.c_str(), textX, dialogY + 45, 14, WHITE);

                // Improved dialog options as clickable buttons with better feedback
                int buttonY = dialogY + 100; // Position buttons relative to centered dialog
                const int buttonHeight = 30;
                const int buttonWidth = 150;
                const int buttonSpacing = 170;
                int startX = dialogX + 25; // Center buttons within dialog

                Vector2 mousePos = GetMousePosition();

                for (int i = 0; i < numDialogOptions; i++) {
                    int buttonX = startX + (i * buttonSpacing);

                    // Enhanced button feedback
                    bool isHovered = (mousePos.y >= buttonY && mousePos.y <= buttonY + buttonHeight &&
                                      mousePos.x >= buttonX && mousePos.x <= buttonX + buttonWidth);

                    // Button background with better visual states
                    Color buttonColor = Fade(BLUE, 0.7f);
                    if (isHovered) {
                        buttonColor = Fade(SKYBLUE, 0.9f); // Highlight when mouse over
                        // Add a subtle glow effect
                        DrawRectangle(buttonX - 2, buttonY - 2, buttonWidth + 4, buttonHeight + 4, Fade(WHITE, 0.3f));
                    }

                    DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
                    DrawRectangleLines(buttonX, buttonY, buttonWidth, buttonHeight, WHITE);

                    // Button text with better positioning
                    DrawText(dialogOptions[i].c_str(), buttonX + 10, buttonY + 8, 12, WHITE);

                    // Add a small indicator if button is hovered
                    if (isHovered) {
                        DrawText(">", buttonX - 15, buttonY + 8, 12, YELLOW);
                    }
                }

                // Instructions (positioned relative to centered dialog)
                int instructionsX = dialogX + (dialogWidth - MeasureText("Click on an option to continue", 12)) / 2;
                DrawText("Click on an option to continue", instructionsX, dialogY + 175, 12, LIGHTGRAY);

                // Debug: Show mouse position for troubleshooting (keep in top right)
                DrawText(TextFormat("Mouse: %.0f, %.0f", mousePos.x, mousePos.y), screenWidth - 150, 30, 10, YELLOW);
            }

            // ===== RIGHT SIDE PANELS =====
            // Move controls help to top-right to avoid left side overlaps
            int controlsPanelX = screenWidth - 350;
            int controlsPanelY = 400;
            int controlsPanelWidth = 330;
            int controlsPanelHeight = 80;

            DrawRectangle(controlsPanelX, controlsPanelY, controlsPanelWidth, controlsPanelHeight, Fade(BLACK, 0.7f));
            DrawRectangleLines(controlsPanelX, controlsPanelY, controlsPanelWidth, controlsPanelHeight, BLUE);
            DrawText("🎮 CONTROLS", controlsPanelX + 10, controlsPanelY + 8, 14, SKYBLUE);
            DrawText("WASD: Move | Mouse: Look | Space: Jump", controlsPanelX + 10, controlsPanelY + 25, 10, LIGHTGRAY);
            DrawText("LMB: Attack | E: Interact | ESC: Toggle mouse", controlsPanelX + 10, controlsPanelY + 38, 10, LIGHTGRAY);
            DrawText("🟢 WORKING | 🟠 UNTESTED | 🔴 BROKEN", controlsPanelX + 10, controlsPanelY + 55, 9, GRAY);

            // Collision warning indicator (moved to right side)
            if (testBuildingCollision) {
                int warningPanelY = controlsPanelY + controlsPanelHeight + 10;
                DrawRectangle(controlsPanelX, warningPanelY, controlsPanelWidth, 30, Fade(RED, 0.8f));
                DrawRectangleLines(controlsPanelX, warningPanelY, controlsPanelWidth, 30, WHITE);
                DrawText("⚠️  WALL COLLISION DETECTED", controlsPanelX + 10, warningPanelY + 8, 11, WHITE);
            }

            // Mouse state indicator (moved to bottom-right)
            const char* mouseStateText = mouseReleased ? "MOUSE FREE" : "MOUSE CAPTURED";
            int mousePanelY = screenHeight - 40;
            DrawRectangle(controlsPanelX, mousePanelY, controlsPanelWidth, 30, Fade(mouseStateColor, 0.7f));
            DrawRectangleLines(controlsPanelX, mousePanelY, controlsPanelWidth, 30, WHITE);
            DrawText(mouseStateText, controlsPanelX + 10, mousePanelY + 8, 12, WHITE);

            // Draw projected labels for buildings and NPCs
            // For buildings
            auto objects = environment.getAllObjects();
            for (const auto& obj : objects) {
                if (auto building = std::dynamic_pointer_cast<Building>(obj)) {
                    Vector3 signPos = building->getDoorPosition();
                    signPos.y += 2.2f;
                    Vector2 screenPos = GetWorldToScreen(signPos, camera);
                    if (screenPos.x > 0 && screenPos.x < screenWidth && screenPos.y > 0 && screenPos.y < screenHeight) {
                        int textWidth = MeasureText(building->getName().c_str(), 20);
                        DrawText(building->getName().c_str(), screenPos.x - textWidth / 2, screenPos.y - 20, 20, BLACK);
                    }
                }
            }

            // For NPCs
            for (int n = 0; n < MAX_NPCS; n++) {
                bool shouldLabelNPC = false;
                if (isInBuilding) {
                    if (currentBuilding == 0 && n == 0) shouldLabelNPC = true;
                    if (currentBuilding == 1 && n == 1) shouldLabelNPC = true;
                }
                if (shouldLabelNPC) {
                    Vector3 labelPos = npcs[n].position;
                    labelPos.y += 2.5f;
                    Vector2 screenPos = GetWorldToScreen(labelPos, camera);
                    if (screenPos.x > 0 && screenPos.x < screenWidth && screenPos.y > 0 && screenPos.y < screenHeight) {
                        int textWidth = MeasureText(npcs[n].name.c_str(), 18);
                        DrawText(npcs[n].name.c_str(), screenPos.x - textWidth / 2, screenPos.y - 20, 18, WHITE);
                    }
                }
            }

        EndDrawing();
    }

    // Keep cursor captured during gameplay, only release on exit
    // Note: We don't need to re-enable cursor during dialog since it should stay captured
    EnableCursor();

    // Final test summary
    std::cout << "\n🏘️  TOWN EXPLORATION TEST SUMMARY:" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << (testMouseCaptured ? "✅" : "❌") << " Mouse Capture: " << (testMouseCaptured ? "WORKING" : "NOT WORKING") << std::endl;
    std::cout << (testWASDMovement ? "✅" : "❌") << " WASD Movement: " << (testWASDMovement ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (testSpaceJump ? "✅" : "❌") << " Space Jump: " << (testSpaceJump ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (testMouseLook ? "✅" : "❌") << " Mouse Look: " << (testMouseLook ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (testMeleeSwing ? "✅" : "❌") << " Longsword Swing: " << (testMeleeSwing ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (testMeleeHitDetection ? "✅" : "❌") << " Melee Hit Detection: " << (testMeleeHitDetection ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (testBuildingEntry ? "✅" : "❌") << " Building Entry: " << (testBuildingEntry ? "WORKING" : "NOT TESTED") << std::endl;
    std::cout << (testNPCInteraction ? "✅" : "❌") << " NPC Interaction: " << (testNPCInteraction ? "WORKING" : "NOT TESTED") << std::endl;

    int workingFeatures = (testMouseCaptured ? 1 : 0) + (testWASDMovement ? 1 : 0) +
                         (testSpaceJump ? 1 : 0) + (testMouseLook ? 1 : 0) +
                         (testMeleeSwing ? 1 : 0) + (testMeleeHitDetection ? 1 : 0) +
                         (testBuildingEntry ? 1 : 0) + (testNPCInteraction ? 1 : 0);

    std::cout << "\n🏆 Final Score: " << score << " points" << std::endl;
    std::cout << "🏘️  Features Working: " << workingFeatures << "/8" << std::endl;
    std::cout << "🗡️  Swings Performed: " << swingsPerformed << std::endl;
    std::cout << "🎯 Melee Hits: " << meleeHits << std::endl;
    std::cout << "🏢 Buildings Visited: " << (testBuildingEntry ? "YES" : "NO") << std::endl;
    std::cout << "👥 NPCs Talked To: " << (testNPCInteraction ? "YES" : "NO") << std::endl;
    std::cout << "💬 Dialog Conversations: " << (isInDialog ? "ACTIVE" : "COMPLETED") << std::endl;

    if (workingFeatures == 8) {
        std::cout << "🎉 ALL FEATURES WORKING PERFECTLY!" << std::endl;
    } else if (workingFeatures >= 6) {
        std::cout << "👍 MOST FEATURES WORKING WELL!" << std::endl;
    } else {
        std::cout << "⚠️  Some features need attention." << std::endl;
    }

    // De-Initialization
    CloseWindow();  // Close window and OpenGL context

    return 0;
}

// Function to start dialog with NPC
void startDialog(int npcIndex) {
    isInDialog = true;
    currentNPC = npcIndex;
    showDialogWindow = true;

    // Release mouse cursor for dialog interaction (only if not already released by user)
    if (!mouseReleased) {
    EnableCursor();
    }

    // Set up dialog based on NPC
    switch (npcIndex) {
        case 0: // Mayor White
            dialogText = "Mayor White: \"Greetings, citizen! Welcome to my office. What brings you here today?\"";
            dialogOptions[0] = "Ask about town affairs";
            dialogOptions[1] = "Request town assistance";
            dialogOptions[2] = "Goodbye";
            numDialogOptions = 3;
            break;

        case 1: // Buster Shoppin
            dialogText = "Buster Shoppin: \"Welcome to my shop! I've got all sorts of goods and supplies for sale.\"";
            dialogOptions[0] = "Browse general supplies";
            dialogOptions[1] = "Ask about special items";
            dialogOptions[2] = "Goodbye";
            numDialogOptions = 3;
            break;
    }
}

// Function to handle dialog option selection
void handleDialogOption(int optionIndex) {
    if (currentNPC == -1) return;

    switch (currentNPC) {
        case 0: // Mayor White
            if (optionIndex == 0) {
                dialogText = "Mayor White: \"Town affairs are going well! We're always looking for capable adventurers to help with local issues.\"";
            } else if (optionIndex == 1) {
                dialogText = "Mayor White: \"What kind of assistance do you need? I can provide information or direct you to the right people.\"";
            } else {
                dialogText = "Mayor White: \"Thank you for visiting. Please come back if you need anything else.\"";
                isInDialog = false;
                showDialogWindow = false;
                // Re-capture mouse cursor for FPS gameplay (only if not released by user)
                if (!mouseReleased) {
                    DisableCursor();
                }
            }
            break;

        case 1: // Buster Shoppin
            if (optionIndex == 0) {
                dialogText = "Buster Shoppin: \"I have food, water, rope, torches, and basic adventuring supplies. What catches your eye?\"";
            } else if (optionIndex == 1) {
                dialogText = "Buster Shoppin: \"I sometimes get rare items from traveling merchants. Check back later for special deals!\"";
            } else {
                dialogText = "Buster Shoppin: \"Thanks for shopping! Come back anytime you need supplies.\"";
                isInDialog = false;
                showDialogWindow = false;
                // Re-capture mouse cursor for FPS gameplay (only if not released by user)
                if (!mouseReleased) {
                    DisableCursor();
                }
            }
            break;
    }

    // Update dialog options for follow-up responses
    if (isInDialog) {
        dialogOptions[0] = "Continue talking";
        dialogOptions[1] = "Ask another question";
        dialogOptions[2] = "Goodbye";
        numDialogOptions = 3;
    }
}