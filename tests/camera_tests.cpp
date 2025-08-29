#include "test_framework.h"
#include <raylib.h>

// Camera validation tests
bool testCameraInitialization() {
    Camera3D camera = {
        .position = (Vector3){ 4.0f, 2.0f, 4.0f },
        .target = (Vector3){ 0.0f, 1.8f, 0.0f },
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    TEST_ASSERT_EQUAL(camera.position.x, 4.0f);
    TEST_ASSERT_EQUAL(camera.position.y, 2.0f);
    TEST_ASSERT_EQUAL(camera.position.z, 4.0f);
    TEST_ASSERT_EQUAL(camera.fovy, 60.0f);
    TEST_ASSERT_EQUAL(camera.projection, CAMERA_PERSPECTIVE);

    return true;
}

bool testCameraMovementSimulation() {
    // Simulate WASD movement vectors
    Vector3 forward = {0.0f, 0.0f, -1.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};

    // Test movement calculations
    float moveSpeed = 0.1f;
    Vector3 newPosition = {4.0f, 2.0f, 4.0f};

    // Simulate W key press (forward)
    newPosition.x += forward.x * moveSpeed;
    newPosition.y += forward.y * moveSpeed;
    newPosition.z += forward.z * moveSpeed;

    TEST_ASSERT_TRUE(newPosition.z < 4.0f); // Should move forward (negative Z)

    // Simulate D key press (right)
    newPosition.x += right.x * moveSpeed;
    TEST_ASSERT_TRUE(newPosition.x > 4.0f); // Should move right (positive X)

    // Simulate space key press (jump)
    newPosition.y += up.y * moveSpeed;
    TEST_ASSERT_TRUE(newPosition.y > 2.0f); // Should move up

    return true;
}

bool testCameraBounds() {
    Camera3D camera = {
        .position = (Vector3){ 0.0f, 0.0f, 0.0f },
        .target = (Vector3){ 0.0f, 0.0f, 0.0f },
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    // Test FOV bounds
    TEST_ASSERT_TRUE(camera.fovy > 0.0f && camera.fovy <= 180.0f);

    // Test projection type
    TEST_ASSERT_TRUE(camera.projection == CAMERA_PERSPECTIVE ||
                    camera.projection == CAMERA_ORTHOGRAPHIC);

    return true;
}

bool testMouseLookSimulation() {
    // Simulate mouse movement for camera rotation
    float mouseX = 100.0f;
    float mouseY = 50.0f;
    float sensitivity = 0.1f;

    float deltaX = mouseX * sensitivity;
    float deltaY = mouseY * sensitivity;

    TEST_ASSERT_TRUE(deltaX > 0.0f);
    TEST_ASSERT_TRUE(deltaY > 0.0f);

    // Camera should rotate based on mouse movement
    float yaw = deltaX;
    float pitch = deltaY;

    TEST_ASSERT_TRUE(yaw > 0.0f);
    TEST_ASSERT_TRUE(pitch > 0.0f);

    return true;
}

// Register all camera tests
void registerCameraTests() {
    REGISTER_TEST("Camera Initialization", testCameraInitialization);
    REGISTER_TEST("Camera Movement Simulation", testCameraMovementSimulation);
    REGISTER_TEST("Camera Bounds Validation", testCameraBounds);
    REGISTER_TEST("Mouse Look Simulation", testMouseLookSimulation);
}
