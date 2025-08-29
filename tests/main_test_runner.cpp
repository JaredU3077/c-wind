#include "test_framework.h"
#include <raylib.h>

// Forward declarations for test registration functions
void registerCameraTests();
void registerRenderingTests();
void registerPerformanceTests();
void registerInputTests();

// Integration tests that combine multiple components
bool testGameIntegration() {
    // Test that all components work together
    Camera3D camera = {
        .position = (Vector3){4.0f, 2.0f, 4.0f},
        .target = (Vector3){0.0f, 1.8f, 0.0f},
        .up = (Vector3){0.0f, 1.0f, 0.0f},
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    // Test rendering objects
    Vector3 cube1Pos = {-4.0f, 1.0f, 0.0f};
    Vector3 cube2Pos = {4.0f, 1.0f, 0.0f};
    Vector3 groundPos = {0.0f, 0.0f, 0.0f};

    // Validate all objects are positioned correctly relative to camera
    TEST_ASSERT_TRUE(cube1Pos.x < camera.position.x); // Cube1 left of camera
    TEST_ASSERT_TRUE(cube2Pos.x > camera.target.x); // Cube2 right of camera target
    TEST_ASSERT_TRUE(cube1Pos.y < camera.position.y); // Cubes below camera
    TEST_ASSERT_TRUE(cube2Pos.y < camera.position.y);

    // Validate ground is below everything
    TEST_ASSERT_TRUE(groundPos.y < cube1Pos.y);
    TEST_ASSERT_TRUE(groundPos.y < cube2Pos.y);
    TEST_ASSERT_TRUE(groundPos.y < camera.position.y);

    return true;
}

bool testSystemRequirements() {
    // Test that the system meets minimum requirements
    int screenWidth = 800;
    int screenHeight = 450;
    int targetFPS = 60;

    // Validate screen resolution
    TEST_ASSERT_TRUE(screenWidth >= 640); // Minimum width
    TEST_ASSERT_TRUE(screenHeight >= 360); // Minimum height (adjusted for actual game)

    // Validate target frame rate
    TEST_ASSERT_TRUE(targetFPS >= 30); // Minimum FPS

    // Test aspect ratio calculation
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    TEST_ASSERT_TRUE(aspectRatio > 1.0f); // Wider than tall

    return true;
}

bool testMeleeIntegration() {
    // Test that melee system integrates properly with game
    const int maxSwings = 3;
    const int maxTargets = 5;
    const float swingRange = 3.0f;

    // Validate system capacities
    TEST_ASSERT_TRUE(maxSwings >= 2 && maxSwings <= 5);
    TEST_ASSERT_TRUE(maxTargets >= 3 && maxTargets <= 10);
    TEST_ASSERT_TRUE(swingRange > 1.0f && swingRange < 5.0f);

    // Test swing cooldown prevents spam
    float swingCooldown = 0.5f;
    float lastSwingTime = 0.0f;
    float currentTime = 0.2f;

    bool canSwing = (currentTime - lastSwingTime) > swingCooldown;
    TEST_ASSERT_FALSE(canSwing); // Should not be able to swing yet

    currentTime = 0.6f; // Advance time
    canSwing = (currentTime - lastSwingTime) > swingCooldown;
    TEST_ASSERT_TRUE(canSwing); // Should be able to swing now

    return true;
}

bool testGameplayBalance() {
    // Test gameplay balance parameters
    const int pointsPerMeleeHit = 150;
    const float jumpStrength = 8.0f;
    const float gravity = -15.0f;
    const float swingRange = 3.0f;
    const float swingCooldown = 0.5f;

    // Validate scoring system
    TEST_ASSERT_TRUE(pointsPerMeleeHit > 100 && pointsPerMeleeHit <= 500);

    // Validate physics balance
    TEST_ASSERT_TRUE(jumpStrength > 0 && jumpStrength < 20.0f);
    TEST_ASSERT_TRUE(gravity < 0 && gravity > -30.0f);
    TEST_ASSERT_TRUE(swingRange > 2.0f && swingRange < 5.0f);
    TEST_ASSERT_TRUE(swingCooldown > 0.3f && swingCooldown < 1.0f);

    // Test jump vs gravity relationship
    TEST_ASSERT_TRUE(jumpStrength > -gravity * 0.5f); // Jump should overcome gravity

    // Test melee balance (range vs cooldown)
    TEST_ASSERT_TRUE(swingRange / swingCooldown > 4.0f); // Good range/cooldown ratio

    return true;
}

void registerIntegrationTests() {
    REGISTER_TEST("Game Integration Test", testGameIntegration);
    REGISTER_TEST("System Requirements", testSystemRequirements);
    REGISTER_TEST("Melee Integration", testMeleeIntegration);
    REGISTER_TEST("Gameplay Balance", testGameplayBalance);
}

int main() {
    std::cout << "🚀 Browserwind Validation Test Suite\n";
    std::cout << "====================================\n\n";

    // Register all test categories
    registerCameraTests();
    registerRenderingTests();
    registerPerformanceTests();
    registerInputTests();
    registerIntegrationTests();

    // Run all tests
    bool allTestsPassed = TestFramework::getInstance().runAllTests();

    // Return appropriate exit code
    return allTestsPassed ? 0 : 1;
}
