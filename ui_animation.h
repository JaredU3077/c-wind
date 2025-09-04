// ui_animation.h - Animation system for UI elements in Browserwind
#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

#include "raylib.h"
#include <functional>
#include <vector>
#include <memory>

namespace UIAnimation {

// ============================================================================
// ANIMATION TYPES AND STRUCTURES
// ============================================================================

enum class AnimationType {
    FADE_IN,
    FADE_OUT,
    SCALE_IN,
    SCALE_OUT,
    SLIDE_IN_LEFT,
    SLIDE_IN_RIGHT,
    SLIDE_IN_TOP,
    SLIDE_IN_BOTTOM,
    BOUNCE,
    PULSE,
    SHAKE,
    GLOW
};

enum class EasingFunction {
    LINEAR,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    EASE_IN_BACK,
    EASE_OUT_BACK,
    EASE_IN_OUT_BACK,
    EASE_IN_BOUNCE,
    EASE_OUT_BOUNCE,
    EASE_IN_OUT_BOUNCE
};

struct AnimationState {
    AnimationType type;
    EasingFunction easing;
    float duration;       // Total animation duration in seconds
    float elapsedTime;    // Time elapsed since animation started
    float startValue;     // Starting value (alpha, scale, position, etc.)
    float endValue;       // Target value
    bool isActive;        // Whether animation is currently running
    bool isLooping;       // Whether animation should loop
    std::function<void(float)> onUpdate;  // Callback with current progress (0-1)
    std::function<void()> onComplete;     // Callback when animation finishes

    AnimationState(AnimationType t = AnimationType::FADE_IN,
                   EasingFunction e = EasingFunction::EASE_OUT_QUAD,
                   float dur = 0.3f, bool loop = false)
        : type(t), easing(e), duration(dur), elapsedTime(0.0f),
          startValue(0.0f), endValue(1.0f), isActive(false), isLooping(loop) {}
};

class AnimationManager {
public:
    static AnimationManager& getInstance() {
        static AnimationManager instance;
        return instance;
    }

    // Animation management
    size_t createAnimation(AnimationState animation);
    void startAnimation(size_t animationId);
    void stopAnimation(size_t animationId);
    void update(float deltaTime);
    void clear();

    // Quick animation methods
    size_t animateFadeIn(float duration = 0.3f, std::function<void(float)> callback = nullptr);
    size_t animateFadeOut(float duration = 0.3f, std::function<void(float)> callback = nullptr);
    size_t animateScaleIn(float duration = 0.4f, std::function<void(float)> callback = nullptr);
    size_t animateSlideInLeft(float duration = 0.3f, std::function<void(float)> callback = nullptr);
    size_t animateSlideInRight(float duration = 0.3f, std::function<void(float)> callback = nullptr);
    size_t animateBounce(float duration = 0.6f, std::function<void(float)> callback = nullptr);
    size_t animatePulse(float duration = 1.0f, bool loop = true, std::function<void(float)> callback = nullptr);

    // Utility functions
    float applyEasing(float t, EasingFunction easing);
    float getAnimationProgress(size_t animationId);

private:
    AnimationManager() = default;
    ~AnimationManager() = default;
    AnimationManager(const AnimationManager&) = delete;
    AnimationManager& operator=(const AnimationManager&) = delete;

    std::vector<AnimationState> animations;
    size_t nextAnimationId = 0;
};

// ============================================================================
// EASING FUNCTIONS
// ============================================================================

float linear(float t);
float easeInQuad(float t);
float easeOutQuad(float t);
float easeInOutQuad(float t);
float easeInCubic(float t);
float easeOutCubic(float t);
float easeInOutCubic(float t);
float easeInBack(float t);
float easeOutBack(float t);
float easeInOutBack(float t);
float easeInBounce(float t);
float easeOutBounce(float t);
float easeInOutBounce(float t);

// ============================================================================
// GLOBAL FUNCTIONS FOR EASY ACCESS
// ============================================================================

size_t animateUIElement(AnimationType type, float duration = 0.3f,
                       std::function<void(float)> callback = nullptr);
void startUIAnimation(size_t animationId);
void stopUIAnimation(size_t animationId);

} // namespace UIAnimation

#endif // UI_ANIMATION_H
