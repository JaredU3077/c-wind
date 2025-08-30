#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "raylib.h"

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