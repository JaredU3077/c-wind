#include "input_manager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// **ENHANCED INPUT MANAGER IMPLEMENTATION**

EnhancedInputManager::EnhancedInputManager() {
    // **INITIALIZE KEY BINDINGS** - Set up default game controls
    setKeyBinding("move_forward", KEY_W);
    setKeyBinding("move_backward", KEY_S);
    setKeyBinding("strafe_left", KEY_A);
    setKeyBinding("strafe_right", KEY_D);
    setKeyBinding("jump", KEY_SPACE);
    setKeyBinding("inventory", KEY_I);
    setKeyBinding("interact", KEY_E);
    setKeyBinding("pause", KEY_ESCAPE);
    setKeyBinding("performance_toggle", KEY_P);
    setKeyBinding("quick_use", KEY_ONE);
    setKeyBinding("testing_panel", KEY_TAB);
    
    std::cout << "Enhanced Input Manager initialized with default key bindings" << std::endl;
}

void EnhancedInputManager::update(float deltaTime) {
    currentTime_ += deltaTime;
    
    if (!inputEnabled_) {
        return;  // Skip input processing if disabled
    }
    
    // **UPDATE KEYBOARD STATE** - Check all tracked keys
    std::vector<int> commonKeys = {
        KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, KEY_I, KEY_E,
        KEY_ESCAPE, KEY_P, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE,
        KEY_TAB  // **ADDED**: TAB key for testing panel
    };
    
    for (int key : commonKeys) {
        updateKeyState(key, keyStates_[key]);
    }
    
    // **UPDATE MOUSE STATE** - Check mouse buttons
    std::vector<int> mouseButtons = { MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE };
    for (int button : mouseButtons) {
        updateMouseState(button, mouseStates_[button]);
    }
    
    // **UPDATE MOUSE POSITION AND DELTA**
    mousePosition_ = GetMousePosition();
    Vector2 rawDelta = GetMouseDelta();
    mouseDelta_ = {rawDelta.x * mouseSensitivity_, rawDelta.y * mouseSensitivity_};
    
    // **SMOOTH MOUSE DELTA** - For better camera feel
    mouseDeltaSmoothed_.x = mouseDeltaSmoothed_.x * (1.0f - MOUSE_SMOOTHING_FACTOR) + 
                           mouseDelta_.x * MOUSE_SMOOTHING_FACTOR;
    mouseDeltaSmoothed_.y = mouseDeltaSmoothed_.y * (1.0f - MOUSE_SMOOTHING_FACTOR) + 
                           mouseDelta_.y * MOUSE_SMOOTHING_FACTOR;
    
    // **CLEAN UP OLD EVENTS** from buffer
    while (!eventBuffer_.empty() && eventBuffer_.size() > MAX_BUFFER_SIZE) {
        eventBuffer_.pop();
    }
}

void EnhancedInputManager::reset() {
    keyStates_.clear();
    mouseStates_.clear();
    clearInputBuffer();
    mouseDelta_ = {0, 0};
    mouseDeltaSmoothed_ = {0, 0};
    std::cout << "Input Manager reset completed" << std::endl;
}

// **KEYBOARD INPUT METHODS**

bool EnhancedInputManager::isKeyPressed(int key) const {
    if (!inputEnabled_ || isKeyDebounced(key)) return false;
    auto it = keyStates_.find(key);
    return it != keyStates_.end() && it->second.isPressed;
}

bool EnhancedInputManager::isKeyReleased(int key) const {
    if (!inputEnabled_) return false;
    auto it = keyStates_.find(key);
    return it != keyStates_.end() && it->second.wasPressed && !it->second.isDown;
}

bool EnhancedInputManager::isKeyDown(int key) const {
    if (!inputEnabled_) return false;
    auto it = keyStates_.find(key);
    return it != keyStates_.end() && it->second.isDown;
}

bool EnhancedInputManager::isKeyUp(int key) const {
    if (!inputEnabled_) return true;
    auto it = keyStates_.find(key);
    return it == keyStates_.end() || !it->second.isDown;
}

float EnhancedInputManager::getKeyHoldDuration(int key) const {
    auto it = keyStates_.find(key);
    return it != keyStates_.end() ? it->second.holdDuration : 0.0f;
}

bool EnhancedInputManager::isKeyRepeating(int key, float interval) const {
    if (!inputEnabled_) return false;
    auto it = keyStates_.find(key);
    if (it == keyStates_.end() || !it->second.isDown) return false;
    
    // Check if key has been held long enough for repeating
    return it->second.holdDuration > 0.5f && 
           fmod(it->second.holdDuration, interval) < GetFrameTime();
}

// **MOUSE INPUT METHODS**

bool EnhancedInputManager::isMouseButtonPressed(int button) const {
    if (!inputEnabled_) return false;
    auto it = mouseStates_.find(button);
    return it != mouseStates_.end() && it->second.isPressed;
}

bool EnhancedInputManager::isMouseButtonReleased(int button) const {
    if (!inputEnabled_) return false;
    auto it = mouseStates_.find(button);
    return it != mouseStates_.end() && it->second.wasPressed && !it->second.isDown;
}

bool EnhancedInputManager::isMouseButtonDown(int button) const {
    if (!inputEnabled_) return false;
    auto it = mouseStates_.find(button);
    return it != mouseStates_.end() && it->second.isDown;
}

Vector2 EnhancedInputManager::getMousePosition() const {
    return mousePosition_;
}

Vector2 EnhancedInputManager::getMouseDelta() const {
    return mouseCaptured_ ? mouseDelta_ : Vector2{0, 0};
}

Vector2 EnhancedInputManager::getMouseDeltaSmoothed() const {
    return mouseCaptured_ ? mouseDeltaSmoothed_ : Vector2{0, 0};
}

// **MOUSE CAPTURE MANAGEMENT**

void EnhancedInputManager::setMouseCaptured(bool captured) {
    if (mouseCaptured_ == captured) return;  // No change needed
    
    mouseCaptured_ = captured;
    if (captured) {
        DisableCursor();
        std::cout << "Mouse captured - look mode enabled" << std::endl;
    } else {
        EnableCursor();
        std::cout << "Mouse released - UI interaction enabled" << std::endl;
    }
    
    // Reset mouse delta when changing capture state
    mouseDelta_ = {0, 0};
    mouseDeltaSmoothed_ = {0, 0};
}

// **INPUT VALIDATION & FILTERING**

void EnhancedInputManager::addKeyDebounce(int key, float debounceTime) {
    keyDebounces_[key] = debounceTime;
}

void EnhancedInputManager::clearKeyDebounces() {
    keyDebounces_.clear();
}

bool EnhancedInputManager::isKeyDebounced(int key) const {
    auto it = keyDebounces_.find(key);
    if (it == keyDebounces_.end()) return false;
    
    auto keyIt = keyStates_.find(key);
    if (keyIt == keyStates_.end()) return false;
    
    return (currentTime_ - keyIt->second.lastPressTime) < it->second;
}

// **INPUT BUFFERING**

void EnhancedInputManager::clearInputBuffer() {
    while (!eventBuffer_.empty()) {
        eventBuffer_.pop();
    }
}

std::queue<InputEvent> EnhancedInputManager::getBufferedEvents() {
    std::queue<InputEvent> result = eventBuffer_;
    clearInputBuffer();  // Clear after reading
    return result;
}

void EnhancedInputManager::bufferInputEvent(InputEvent::Type type, int code) {
    if (!bufferingEnabled_) return;
    
    eventBuffer_.push(InputEvent(type, code, currentTime_));
    
    // Prevent buffer overflow
    while (eventBuffer_.size() > MAX_BUFFER_SIZE) {
        eventBuffer_.pop();
    }
}

// **KEY BINDING SYSTEM**

void EnhancedInputManager::setKeyBinding(const std::string& action, int key) {
    keyBindings_[action] = key;
}

int EnhancedInputManager::getKeyBinding(const std::string& action) const {
    auto it = keyBindings_.find(action);
    return it != keyBindings_.end() ? it->second : -1;
}

bool EnhancedInputManager::isActionPressed(const std::string& action) const {
    int key = getKeyBinding(action);
    if (action == "testing_panel") {
        // Debug TAB key detection
        static int debugCounter = 0;
        if (debugCounter++ % 60 == 0) {  // Log every 60 frames
            auto it = keyStates_.find(key);
            bool keyPressed = it != keyStates_.end() && it->second.isPressed;
            std::cout << "TAB DEBUG: Key=" << key << ", Found=" << (it != keyStates_.end()) << ", Pressed=" << keyPressed << std::endl;
        }
    }
    return key != -1 && isKeyPressed(key);
}

bool EnhancedInputManager::isActionDown(const std::string& action) const {
    int key = getKeyBinding(action);
    return key != -1 && isKeyDown(key);
}

// **DEBUG & DIAGNOSTICS**

void EnhancedInputManager::printInputState() const {
    std::cout << "=== INPUT MANAGER STATE ===" << std::endl;
    std::cout << "Input Enabled: " << (inputEnabled_ ? "YES" : "NO") << std::endl;
    std::cout << "Mouse Captured: " << (mouseCaptured_ ? "YES" : "NO") << std::endl;
    std::cout << "Active Keys: " << getActiveKeyCount() << std::endl;
    std::cout << "Mouse Delta: (" << mouseDelta_.x << ", " << mouseDelta_.y << ")" << std::endl;
    std::cout << "Mouse Position: (" << mousePosition_.x << ", " << mousePosition_.y << ")" << std::endl;
}

int EnhancedInputManager::getActiveKeyCount() const {
    int count = 0;
    for (const auto& pair : keyStates_) {
        if (pair.second.isDown) count++;
    }
    return count;
}

std::string EnhancedInputManager::getInputSummary() const {
    return "Keys: " + std::to_string(getActiveKeyCount()) + 
           " | Mouse: " + (mouseCaptured_ ? "Captured" : "Free") +
           " | Input: " + (inputEnabled_ ? "Enabled" : "Disabled");
}

// **HELPER FUNCTIONS**

void EnhancedInputManager::updateKeyState(int key, KeyState& state) {
    // Store previous state
    state.wasPressed = state.isDown;
    
    // Get current raylib state
    bool currentlyDown = IsKeyDown(key);
    bool justPressed = IsKeyPressed(key);
    
    // Update state
    state.isPressed = justPressed && !isKeyDebounced(key);
    state.isDown = currentlyDown;
    
    // Track hold duration
    if (state.isDown) {
        if (!state.wasPressed) {
            // Just started pressing
            state.lastPressTime = currentTime_;
            state.holdDuration = 0.0f;
            
            // Buffer the event
            bufferInputEvent(InputEvent::KEY_PRESS, key);
        } else {
            // Continue holding
            state.holdDuration = currentTime_ - state.lastPressTime;
        }
    } else if (state.wasPressed) {
        // Just released
        state.holdDuration = 0.0f;
        bufferInputEvent(InputEvent::KEY_RELEASE, key);
    }
}

void EnhancedInputManager::updateMouseState(int button, MouseState& state) {
    // Store previous state
    state.wasPressed = state.isDown;
    
    // Get current raylib state
    bool currentlyDown = IsMouseButtonDown(button);
    bool justPressed = IsMouseButtonPressed(button);
    
    // Update state
    state.isPressed = justPressed;
    state.isDown = currentlyDown;
    
    // Track timing
    if (state.isPressed) {
        state.lastPressTime = currentTime_;
        bufferInputEvent(InputEvent::MOUSE_PRESS, button);
    } else if (state.wasPressed && !state.isDown) {
        bufferInputEvent(InputEvent::MOUSE_RELEASE, button);
    }
}

// **LEGACY COMPATIBILITY IMPLEMENTATION** - Keep original InputManager working

void InputManager::update() {
    mouseDelta_ = GetMouseDelta();
}

bool InputManager::isKeyPressed(int key) const {
    return IsKeyPressed(key);
}

bool InputManager::isKeyDown(int key) const {
    return IsKeyDown(key);
}

bool InputManager::isMouseButtonPressed(int button) const {
    return IsMouseButtonPressed(button);
}

Vector2 InputManager::getMouseDelta() const {
    return mouseDelta_;
}

void InputManager::setMouseCaptured(bool captured) {
    mouseCaptured_ = captured;
    if (captured) {
        DisableCursor();
    } else {
        EnableCursor();
    }
}