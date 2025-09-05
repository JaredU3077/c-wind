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
    current_time_ += deltaTime;
    
    if (!input_enabled_) {
        return;  // Skip input processing if disabled
    }
    
    // **UPDATE KEYBOARD STATE** - Check all tracked keys
    std::vector<int> commonKeys = {
        KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, KEY_I, KEY_E,
        KEY_ESCAPE, KEY_P, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE,
        KEY_TAB  // **ADDED**: TAB key for testing panel
    };
    
    for (int key : commonKeys) {
        updateKeyState(key, key_states_[key]);
    }
    
    // **UPDATE MOUSE STATE** - Check mouse buttons
    std::vector<int> mouseButtons = { MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE };
    for (int button : mouseButtons) {
        updateMouseState(button, mouse_states_[button]);
    }
    
    // **UPDATE MOUSE POSITION AND DELTA**
    mouse_position_ = GetMousePosition();
    Vector2 rawDelta = GetMouseDelta();
    mouse_delta_ = {rawDelta.x * mouse_sensitivity_, rawDelta.y * mouse_sensitivity_};

    // Debug mouse input every 60 frames
    static int frameCounter = 0;
    if (frameCounter++ % 60 == 0) {
        std::cout << "MOUSE DEBUG: Pos(" << mouse_position_.x << "," << mouse_position_.y
                  << ") Delta(" << mouse_delta_.x << "," << mouse_delta_.y
                  << ") RawDelta(" << rawDelta.x << "," << rawDelta.y
                  << ") Captured: " << (mouse_captured_ ? "YES" : "NO") << std::endl;
    }

    // Debug mouse delta right before return
    static int returnCounter = 0;
    if (returnCounter++ % 120 == 0) {
        std::cout << "MOUSE RETURN: Final delta (" << mouse_delta_.x << "," << mouse_delta_.y << ")" << std::endl;
    }

    // Debug camera angle changes
    static int cameraFrameCounter = 0;
    if (cameraFrameCounter++ % 120 == 0 && mouse_captured_) {
        std::cout << "CAMERA: Mouse should rotate camera view (cursor hidden for FPS controls)" << std::endl;
    }
    
    // **SMOOTH MOUSE DELTA** - For better camera feel
    mouse_delta_smoothed_.x = mouse_delta_smoothed_.x * (1.0f - MOUSE_SMOOTHING_FACTOR) + 
                           mouse_delta_.x * MOUSE_SMOOTHING_FACTOR;
    mouse_delta_smoothed_.y = mouse_delta_smoothed_.y * (1.0f - MOUSE_SMOOTHING_FACTOR) + 
                           mouse_delta_.y * MOUSE_SMOOTHING_FACTOR;
    
    // **NEW: DISPATCH ACTION CALLBACKS**
    for (const auto& pair : key_bindings_) {
        const std::string& action = pair.first;
        if (isActionPressed(action)) {
            auto cbIt = action_callbacks_.find(action);
            if (cbIt != action_callbacks_.end()) {
                cbIt->second();  // Call the callback
            }
        }
    }
    
    // **CLEAN UP OLD EVENTS** from buffer
    while (!event_buffer_.empty() && event_buffer_.size() > MAX_BUFFER_SIZE) {
        event_buffer_.pop();
    }
}

void EnhancedInputManager::reset() {
    key_states_.clear();
    mouse_states_.clear();
    clearInputBuffer();
    mouse_delta_ = {0, 0};
    mouse_delta_smoothed_ = {0, 0};
    std::cout << "Input Manager reset completed" << std::endl;
}

// **KEYBOARD INPUT METHODS**

bool EnhancedInputManager::isKeyPressed(int key) const {
    if (!input_enabled_ || isKeyDebounced(key)) return false;
    auto it = key_states_.find(key);
    return it != key_states_.end() && it->second.isPressed;
}

bool EnhancedInputManager::isKeyReleased(int key) const {
    if (!input_enabled_) return false;
    auto it = key_states_.find(key);
    return it != key_states_.end() && it->second.wasPressed && !it->second.isDown;
}

bool EnhancedInputManager::isKeyDown(int key) const {
    if (!input_enabled_) return false;
    auto it = key_states_.find(key);
    return it != key_states_.end() && it->second.isDown;
}

bool EnhancedInputManager::isKeyUp(int key) const {
    if (!input_enabled_) return true;
    auto it = key_states_.find(key);
    return it == key_states_.end() || !it->second.isDown;
}

float EnhancedInputManager::getKeyHoldDuration(int key) const {
    auto it = key_states_.find(key);
    return it != key_states_.end() ? it->second.holdDuration : 0.0f;
}

bool EnhancedInputManager::isKeyRepeating(int key, float interval) const {
    if (!input_enabled_) return false;
    auto it = key_states_.find(key);
    if (it == key_states_.end() || !it->second.isDown) return false;
    
    // Check if key has been held long enough for repeating
    return it->second.holdDuration > 0.5f && 
           std::fmod(it->second.holdDuration, interval) < GetFrameTime();
}

// **MOUSE INPUT METHODS**

bool EnhancedInputManager::isMouseButtonPressed(int button) const {
    if (!input_enabled_) return false;
    auto it = mouse_states_.find(button);
    return it != mouse_states_.end() && it->second.isPressed;
}

bool EnhancedInputManager::isMouseButtonReleased(int button) const {
    if (!input_enabled_) return false;
    auto it = mouse_states_.find(button);
    return it != mouse_states_.end() && it->second.wasPressed && !it->second.isDown;
}

bool EnhancedInputManager::isMouseButtonDown(int button) const {
    if (!input_enabled_) return false;
    auto it = mouse_states_.find(button);
    return it != mouse_states_.end() && it->second.isDown;
}

Vector2 EnhancedInputManager::getMousePosition() const {
    return mouse_position_;
}

Vector2 EnhancedInputManager::getMouseDelta() const {
    return mouse_captured_ ? mouse_delta_ : Vector2{0, 0};
}

Vector2 EnhancedInputManager::getMouseDeltaSmoothed() const {
    return mouse_captured_ ? mouse_delta_smoothed_ : Vector2{0, 0};
}

float EnhancedInputManager::getMouseSensitivity() const {
    return mouse_sensitivity_;
}

void EnhancedInputManager::setMouseSensitivity(float sensitivity) {
    mouse_sensitivity_ = sensitivity;
}

// **MOUSE CAPTURE MANAGEMENT**

void EnhancedInputManager::setMouseCaptured(bool captured) {
    if (mouse_captured_ == captured) return;  // No change needed
    
    mouse_captured_ = captured;
    if (captured) {
        DisableCursor();
        std::cout << "Mouse captured - look mode enabled" << std::endl;
    } else {
        EnableCursor();
        std::cout << "Mouse released - UI interaction enabled" << std::endl;
    }
    
    // Reset mouse delta when changing capture state
    mouse_delta_ = {0, 0};
    mouse_delta_smoothed_ = {0, 0};
}

bool EnhancedInputManager::isMouseCaptured() const {
    return mouse_captured_;
}

void EnhancedInputManager::toggleMouseCapture() {
    setMouseCaptured(!mouse_captured_);
}

// **INPUT VALIDATION & FILTERING**

void EnhancedInputManager::setInputEnabled(bool enabled) {
    input_enabled_ = enabled;
}

bool EnhancedInputManager::isInputEnabled() const {
    return input_enabled_;
}

void EnhancedInputManager::addKeyDebounce(int key, float debounceTime) {
    key_debounces_[key] = debounceTime;
}

void EnhancedInputManager::clearKeyDebounces() {
    key_debounces_.clear();
}

bool EnhancedInputManager::isKeyDebounced(int key) const {
    auto it = key_debounces_.find(key);
    if (it == key_debounces_.end()) return false;
    
    auto keyIt = key_states_.find(key);
    if (keyIt == key_states_.end()) return false;
    
    return (current_time_ - keyIt->second.lastPressTime) < it->second;
}

// **INPUT BUFFERING**

void EnhancedInputManager::enableInputBuffering(bool enabled) {
    buffering_enabled_ = enabled;
}

void EnhancedInputManager::clearInputBuffer() {
    while (!event_buffer_.empty()) {
        event_buffer_.pop();
    }
}

std::queue<InputEvent> EnhancedInputManager::getBufferedEvents() {
    std::queue<InputEvent> result = event_buffer_;
    clearInputBuffer();  // Clear after reading
    return result;
}

void EnhancedInputManager::bufferInputEvent(InputEvent::Type type, int code) {
    if (!buffering_enabled_) return;
    
    event_buffer_.push(InputEvent(type, code, current_time_));
    
    // Prevent buffer overflow
    while (event_buffer_.size() > MAX_BUFFER_SIZE) {
        event_buffer_.pop();
    }
}

// **KEY BINDING SYSTEM**

void EnhancedInputManager::setKeyBinding(const std::string& action, int key) {
    key_bindings_[action] = key;
}

int EnhancedInputManager::getKeyBinding(const std::string& action) const {
    auto it = key_bindings_.find(action);
    return it != key_bindings_.end() ? it->second : -1;
}

bool EnhancedInputManager::isActionPressed(const std::string& action) const {
    int key = getKeyBinding(action);
    return key != -1 && isKeyPressed(key);
}

bool EnhancedInputManager::isActionDown(const std::string& action) const {
    int key = getKeyBinding(action);
    return key != -1 && isKeyDown(key);
}

// **NEW METHODS**
void EnhancedInputManager::registerActionCallback(const std::string& action, std::function<void()> callback) {
    action_callbacks_[action] = callback;
    std::cout << "Registered callback for action: " << action << std::endl;
}

void EnhancedInputManager::unregisterActionCallback(const std::string& action) {
    action_callbacks_.erase(action);
}

// **DEBUG & DIAGNOSTICS**

void EnhancedInputManager::printInputState() const {
    std::cout << "=== INPUT MANAGER STATE ===" << std::endl;
    std::cout << "Input Enabled: " << (input_enabled_ ? "YES" : "NO") << std::endl;
    std::cout << "Mouse Captured: " << (mouse_captured_ ? "YES" : "NO") << std::endl;
    std::cout << "Active Keys: " << getActiveKeyCount() << std::endl;
    std::cout << "Mouse Delta: (" << mouse_delta_.x << ", " << mouse_delta_.y << ")" << std::endl;
    std::cout << "Mouse Position: (" << mouse_position_.x << ", " << mouse_position_.y << ")" << std::endl;
}

int EnhancedInputManager::getActiveKeyCount() const {
    int count = 0;
    for (const auto& pair : key_states_) {
        if (pair.second.isDown) count++;
    }
    return count;
}

std::string EnhancedInputManager::getInputSummary() const {
    return "Keys: " + std::to_string(getActiveKeyCount()) + 
           " | Mouse: " + (mouse_captured_ ? "Captured" : "Free") +
           " | Input: " + (input_enabled_ ? "Enabled" : "Disabled");
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
            state.lastPressTime = current_time_;
            state.holdDuration = 0.0f;
            
            // Buffer the event
            bufferInputEvent(InputEvent::KEY_PRESS, key);
        } else {
            // Continue holding
            state.holdDuration = current_time_ - state.lastPressTime;
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
        state.lastPressTime = current_time_;
        bufferInputEvent(InputEvent::MOUSE_PRESS, button);
    } else if (state.wasPressed && !state.isDown) {
        bufferInputEvent(InputEvent::MOUSE_RELEASE, button);
    }
}

// **LEGACY COMPATIBILITY IMPLEMENTATION** - Keep original InputManager working

void InputManager::update() {
    mouse_delta_ = GetMouseDelta();
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
    return mouse_delta_;
}

void InputManager::setMouseCaptured(bool captured) {
    mouse_captured_ = captured;
    if (captured) {
        DisableCursor();
    } else {
        EnableCursor();
    }
}