#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "raylib.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <string>
#include <functional>  // For std::function

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

/// \brief Enhanced input manager for handling keyboard and mouse.
class EnhancedInputManager {
public:
    // **CORE LIFECYCLE**
    /// \brief Constructor.
    EnhancedInputManager();

    /// \brief Updates input state.
    /// \param deltaTime Delta time.
    void update(float deltaTime);

    /// \brief Resets input state.
    void reset();
    
    // **KEYBOARD INPUT** - Enhanced with caching and validation
    /// \brief Checks if key pressed this frame.
    /// \param key Key code.
    /// \return True if pressed.
    bool isKeyPressed(int key) const;           

    /// \brief Checks if key released this frame.
    /// \param key Key code.
    /// \return True if released.
    bool isKeyReleased(int key) const;          

    /// \brief Checks if key is down.
    /// \param key Key code.
    /// \return True if down.
    bool isKeyDown(int key) const;              

    /// \brief Checks if key is up.
    /// \param key Key code.
    /// \return True if up.
    bool isKeyUp(int key) const;                

    /// \brief Gets key hold duration.
    /// \param key Key code.
    /// \return Hold duration.
    float getKeyHoldDuration(int key) const;    

    /// \brief Checks if key repeating.
    /// \param key Key code.
    /// \param interval Repeat interval.
    /// \return True if repeating.
    bool isKeyRepeating(int key, float interval = 0.3f) const;  
    
    // **MOUSE INPUT** - Enhanced with state tracking
    /// \brief Checks if mouse button pressed.
    /// \param button Button.
    /// \return True if pressed.
    bool isMouseButtonPressed(int button) const;

    /// \brief Checks if mouse button released.
    /// \param button Button.
    /// \return True if released.
    bool isMouseButtonReleased(int button) const;

    /// \brief Checks if mouse button down.
    /// \param button Button.
    /// \return True if down.
    bool isMouseButtonDown(int button) const;

    /// \brief Gets mouse position.
    /// \return Mouse position.
    Vector2 getMousePosition() const;

    /// \brief Gets mouse delta.
    /// \return Mouse delta.
    Vector2 getMouseDelta() const;

    /// \brief Gets smoothed mouse delta.
    /// \return Smoothed delta.
    Vector2 getMouseDeltaSmoothed() const;      

    /// \brief Gets mouse sensitivity.
    /// \return Sensitivity.
    float getMouseSensitivity() const;

    /// \brief Sets mouse sensitivity.
    /// \param sensitivity Sensitivity.
    void setMouseSensitivity(float sensitivity);
    
    // **MOUSE CAPTURE MANAGEMENT** - Centralized and enhanced
    /// \brief Sets mouse captured.
    /// \param captured Captured state.
    void setMouseCaptured(bool captured);

    /// \brief Checks if mouse captured.
    /// \return True if captured.
    bool isMouseCaptured() const;

    /// \brief Toggles mouse capture.
    void toggleMouseCapture();
    
    // **INPUT VALIDATION & FILTERING**
    /// \brief Sets input enabled.
    /// \param enabled Enabled state.
    void setInputEnabled(bool enabled);

    /// \brief Checks if input enabled.
    /// \return True if enabled.
    bool isInputEnabled() const;

    /// \brief Adds key debounce.
    /// \param key Key.
    /// \param debounceTime Debounce time.
    void addKeyDebounce(int key, float debounceTime = 0.1f);

    /// \brief Clears key debounces.
    void clearKeyDebounces();
    
    // **INPUT BUFFERING** - For responsive input
    /// \brief Enables input buffering.
    /// \param enabled Enabled state.
    void enableInputBuffering(bool enabled);

    /// \brief Gets buffered events.
    /// \return Buffered events.
    std::queue<InputEvent> getBufferedEvents();

    /// \brief Clears input buffer.
    void clearInputBuffer();
    
    // **ADVANCED FEATURES**
    /// \brief Sets key binding.
    /// \param action Action name.
    /// \param key Key code.
    void setKeyBinding(const std::string& action, int key);

    /// \brief Gets key binding.
    /// \param action Action name.
    /// \return Key code.
    int getKeyBinding(const std::string& action) const;

    /// \brief Checks if action pressed.
    /// \param action Action name.
    /// \return True if pressed.
    bool isActionPressed(const std::string& action) const;

    /// \brief Checks if action down.
    /// \param action Action name.
    /// \return True if down.
    bool isActionDown(const std::string& action) const;

    // **NEW: ACTION CALLBACKS** - Event-driven dispatch
    /// \brief Registers action callback.
    /// \param action Action name.
    /// \param callback Callback.
    void registerActionCallback(const std::string& action, std::function<void()> callback);

    /// \brief Unregisters action callback.
    /// \param action Action name.
    void unregisterActionCallback(const std::string& action);
    
    // **DEBUG & DIAGNOSTICS**
    /// \brief Prints input state.
    void printInputState() const;

    /// \brief Gets active key count.
    /// \return Active keys.
    int getActiveKeyCount() const;

    /// \brief Gets input summary.
    /// \return Summary string.
    std::string getInputSummary() const;

private:
    // **STATE MANAGEMENT**
    std::unordered_map<int, KeyState> key_states_;
    std::unordered_map<int, MouseState> mouse_states_;
    std::unordered_map<int, float> key_debounces_;  // Key -> debounce time
    std::unordered_map<std::string, int> key_bindings_;  // Action -> Key
    
    // **MOUSE STATE**
    bool mouse_captured_ = true;
    Vector2 mouse_position_ = {0, 0};
    Vector2 mouse_delta_ = {0, 0};
    Vector2 mouse_delta_smoothed_ = {0, 0};
    float mouse_sensitivity_ = 1.0f;
    
    // **INPUT BUFFERING**
    bool buffering_enabled_ = false;
    std::queue<InputEvent> event_buffer_;
    static constexpr int MAX_BUFFER_SIZE = 64;
    
    // **SYSTEM STATE**
    bool input_enabled_ = true;
    float current_time_ = 0.0f;
    
    // **SMOOTHING**
    static constexpr float MOUSE_SMOOTHING_FACTOR = 0.2f;
    
    // **NEW: CALLBACK STORAGE**
    std::unordered_map<std::string, std::function<void()>> action_callbacks_;
    
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
    bool isMouseCaptured() const { return mouse_captured_; }
private:
    bool mouse_captured_ = true;
    Vector2 mouse_delta_ = {0, 0};
};

#endif