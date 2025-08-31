#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "raylib.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <string>

// **ENHANCED INPUT MANAGER SYSTEM** - Centralized input with state caching and validation

struct InputEvent {
    enum Type { KEY_PRESS, KEY_RELEASE, MOUSE_PRESS, MOUSE_RELEASE };
    Type type;
    int code;  // Key code or mouse button
    float timestamp;
    
    InputEvent(Type t, int c, float ts) : type(t), code(c), timestamp(ts) {}
};

struct KeyState {
    bool isPressed = false;     // This frame
    bool wasPressed = false;    // Previous frame  
    bool isDown = false;        // Held down
    float lastPressTime = 0.0f; // For debouncing
    float holdDuration = 0.0f;  // How long held
};

struct MouseState {
    bool isPressed = false;
    bool wasPressed = false;
    bool isDown = false;
    float lastPressTime = 0.0f;
};

class EnhancedInputManager {
public:
    // **CORE LIFECYCLE**
    EnhancedInputManager();
    void update(float deltaTime);
    void reset();
    
    // **KEYBOARD INPUT** - Enhanced with caching and validation
    bool isKeyPressed(int key) const;           // Just pressed this frame
    bool isKeyReleased(int key) const;          // Just released this frame
    bool isKeyDown(int key) const;              // Currently held down
    bool isKeyUp(int key) const;                // Currently not pressed
    float getKeyHoldDuration(int key) const;    // How long key has been held
    bool isKeyRepeating(int key, float interval = 0.3f) const;  // For repeat actions
    
    // **MOUSE INPUT** - Enhanced with state tracking
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    bool isMouseButtonDown(int button) const;
    Vector2 getMousePosition() const;
    Vector2 getMouseDelta() const;
    Vector2 getMouseDeltaSmoothed() const;      // Smoothed for better feel
    float getMouseSensitivity() const { return mouseSensitivity_; }
    void setMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
    
    // **MOUSE CAPTURE MANAGEMENT** - Centralized and enhanced
    void setMouseCaptured(bool captured);
    bool isMouseCaptured() const { return mouseCaptured_; }
    void toggleMouseCapture() { setMouseCaptured(!mouseCaptured_); }
    
    // **INPUT VALIDATION & FILTERING**
    void setInputEnabled(bool enabled) { inputEnabled_ = enabled; }
    bool isInputEnabled() const { return inputEnabled_; }
    void addKeyDebounce(int key, float debounceTime = 0.1f);
    void clearKeyDebounces();
    
    // **INPUT BUFFERING** - For responsive input
    void enableInputBuffering(bool enabled) { bufferingEnabled_ = enabled; }
    std::queue<InputEvent> getBufferedEvents();
    void clearInputBuffer();
    
    // **ADVANCED FEATURES**
    void setKeyBinding(const std::string& action, int key);
    int getKeyBinding(const std::string& action) const;
    bool isActionPressed(const std::string& action) const;
    bool isActionDown(const std::string& action) const;
    
    // **DEBUG & DIAGNOSTICS**
    void printInputState() const;
    int getActiveKeyCount() const;
    std::string getInputSummary() const;

private:
    // **STATE MANAGEMENT**
    std::unordered_map<int, KeyState> keyStates_;
    std::unordered_map<int, MouseState> mouseStates_;
    std::unordered_map<int, float> keyDebounces_;  // Key -> debounce time
    std::unordered_map<std::string, int> keyBindings_;  // Action -> Key
    
    // **MOUSE STATE**
    bool mouseCaptured_ = true;
    Vector2 mousePosition_ = {0, 0};
    Vector2 mouseDelta_ = {0, 0};
    Vector2 mouseDeltaSmoothed_ = {0, 0};
    float mouseSensitivity_ = 1.0f;
    
    // **INPUT BUFFERING**
    bool bufferingEnabled_ = false;
    std::queue<InputEvent> eventBuffer_;
    static constexpr int MAX_BUFFER_SIZE = 64;
    
    // **SYSTEM STATE**
    bool inputEnabled_ = true;
    float currentTime_ = 0.0f;
    
    // **SMOOTHING**
    static constexpr float MOUSE_SMOOTHING_FACTOR = 0.2f;
    
    // **HELPER FUNCTIONS**
    void updateKeyState(int key, KeyState& state);
    void updateMouseState(int button, MouseState& state);
    void bufferInputEvent(InputEvent::Type type, int code);
    bool isKeyDebounced(int key) const;
};

// **LEGACY COMPATIBILITY** - Keep original InputManager for gradual migration
class InputManager {
public:
    void update();
    bool isKeyPressed(int key) const;
    bool isKeyDown(int key) const;
    bool isMouseButtonPressed(int button) const;
    Vector2 getMouseDelta() const;
    void setMouseCaptured(bool captured);
    bool isMouseCaptured() const { return mouseCaptured_; }
private:
    bool mouseCaptured_ = true;
    Vector2 mouseDelta_ = {0, 0};
};

#endif