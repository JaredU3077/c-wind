#include "input_manager.h"

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