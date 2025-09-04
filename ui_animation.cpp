// ui_animation.cpp - Implementation of the animation system
#include "ui_animation.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace UIAnimation {

// ============================================================================
// EASING FUNCTIONS IMPLEMENTATION
// ============================================================================

float linear(float t) {
    return t;
}

float easeInQuad(float t) {
    return t * t;
}

float easeOutQuad(float t) {
    return t * (2 - t);
}

float easeInOutQuad(float t) {
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

float easeInCubic(float t) {
    return t * t * t;
}

float easeOutCubic(float t) {
    float f = (t - 1);
    return f * f * f + 1;
}

float easeInOutCubic(float t) {
    return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

float easeInBack(float t) {
    float s = 1.70158f;
    return t * t * ((s + 1) * t - s);
}

float easeOutBack(float t) {
    float s = 1.70158f;
    t -= 1.0f;  // Decrement t first
    return t * t * ((s + 1) * t + s) + 1;
}

float easeInOutBack(float t) {
    float s = 1.70158f * 1.525f;
    if (t < 0.5f) {
        return (t * t * ((s + 1) * t - s)) * 2;
    } else {
        t -= 1.0f;  // Decrement t first
        return (t * t * ((s + 1) * t + s) + 2) * 0.5f;
    }
}

float easeOutBounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

float easeInBounce(float t) {
    return 1.0f - easeOutBounce(1.0f - t);
}

float easeInOutBounce(float t) {
    return t < 0.5f ? easeInBounce(t * 2) * 0.5f : easeOutBounce(t * 2 - 1) * 0.5f + 0.5f;
}

// ============================================================================
// ANIMATION MANAGER IMPLEMENTATION
// ============================================================================

size_t AnimationManager::createAnimation(AnimationState animation) {
    animation.isActive = false;
    animations.push_back(animation);
    return nextAnimationId++;
}

void AnimationManager::startAnimation(size_t animationId) {
    if (animationId < animations.size()) {
        animations[animationId].isActive = true;
        animations[animationId].elapsedTime = 0.0f;
    }
}

void AnimationManager::stopAnimation(size_t animationId) {
    if (animationId < animations.size()) {
        animations[animationId].isActive = false;
        if (animations[animationId].onComplete) {
            animations[animationId].onComplete();
        }
    }
}

void AnimationManager::update(float deltaTime) {
    for (size_t i = 0; i < animations.size(); ++i) {
        AnimationState& animation = animations[i];

        if (!animation.isActive) continue;

        animation.elapsedTime += deltaTime;

        // Handle looping animations
        if (animation.isLooping && animation.elapsedTime >= animation.duration) {
            animation.elapsedTime = 0.0f;
        }
        // Handle completion for non-looping animations
        else if (!animation.isLooping && animation.elapsedTime >= animation.duration) {
            animation.elapsedTime = animation.duration;
            animation.isActive = false;
            if (animation.onComplete) {
                animation.onComplete();
            }
        }

        // Calculate progress and apply easing
        float progress = std::min(animation.elapsedTime / animation.duration, 1.0f);
        float easedProgress = applyEasing(progress, animation.easing);

        // Calculate current value
        float currentValue = animation.startValue +
                           (animation.endValue - animation.startValue) * easedProgress;

        // Call update callback
        if (animation.onUpdate) {
            animation.onUpdate(currentValue);
        }
    }
}

void AnimationManager::clear() {
    animations.clear();
    nextAnimationId = 0;
}

size_t AnimationManager::animateFadeIn(float duration, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::FADE_IN, EasingFunction::EASE_OUT_QUAD, duration);
    animation.startValue = 0.0f;
    animation.endValue = 1.0f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

size_t AnimationManager::animateFadeOut(float duration, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::FADE_OUT, EasingFunction::EASE_IN_QUAD, duration);
    animation.startValue = 1.0f;
    animation.endValue = 0.0f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

size_t AnimationManager::animateScaleIn(float duration, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::SCALE_IN, EasingFunction::EASE_OUT_BACK, duration);
    animation.startValue = 0.0f;
    animation.endValue = 1.0f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

size_t AnimationManager::animateSlideInLeft(float duration, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::SLIDE_IN_LEFT, EasingFunction::EASE_OUT_QUAD, duration);
    animation.startValue = -1.0f;
    animation.endValue = 0.0f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

size_t AnimationManager::animateSlideInRight(float duration, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::SLIDE_IN_RIGHT, EasingFunction::EASE_OUT_QUAD, duration);
    animation.startValue = 1.0f;
    animation.endValue = 0.0f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

size_t AnimationManager::animateBounce(float duration, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::BOUNCE, EasingFunction::EASE_OUT_BOUNCE, duration);
    animation.startValue = 0.0f;
    animation.endValue = 1.0f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

size_t AnimationManager::animatePulse(float duration, bool loop, std::function<void(float)> callback) {
    AnimationState animation(AnimationType::PULSE, EasingFunction::EASE_IN_OUT_QUAD, duration, loop);
    animation.startValue = 1.0f;
    animation.endValue = 1.2f;
    animation.onUpdate = callback;
    return createAnimation(animation);
}

float AnimationManager::applyEasing(float t, EasingFunction easing) {
    switch (easing) {
        case EasingFunction::LINEAR: return linear(t);
        case EasingFunction::EASE_IN_QUAD: return easeInQuad(t);
        case EasingFunction::EASE_OUT_QUAD: return easeOutQuad(t);
        case EasingFunction::EASE_IN_OUT_QUAD: return easeInOutQuad(t);
        case EasingFunction::EASE_IN_CUBIC: return easeInCubic(t);
        case EasingFunction::EASE_OUT_CUBIC: return easeOutCubic(t);
        case EasingFunction::EASE_IN_OUT_CUBIC: return easeInOutCubic(t);
        case EasingFunction::EASE_IN_BACK: return easeInBack(t);
        case EasingFunction::EASE_OUT_BACK: return easeOutBack(t);
        case EasingFunction::EASE_IN_OUT_BACK: return easeInOutBack(t);
        case EasingFunction::EASE_IN_BOUNCE: return easeInBounce(t);
        case EasingFunction::EASE_OUT_BOUNCE: return easeOutBounce(t);
        case EasingFunction::EASE_IN_OUT_BOUNCE: return easeInOutBounce(t);
        default: return linear(t);
    }
}

float AnimationManager::getAnimationProgress(size_t animationId) {
    if (animationId >= animations.size()) return 0.0f;

    const AnimationState& animation = animations[animationId];
    if (!animation.isActive) return 1.0f;

    return std::min(animation.elapsedTime / animation.duration, 1.0f);
}

// ============================================================================
// GLOBAL FUNCTIONS FOR EASY ACCESS
// ============================================================================

size_t animateUIElement(AnimationType type, float duration, std::function<void(float)> callback) {
    switch (type) {
        case AnimationType::FADE_IN:
            return AnimationManager::getInstance().animateFadeIn(duration, callback);
        case AnimationType::FADE_OUT:
            return AnimationManager::getInstance().animateFadeOut(duration, callback);
        case AnimationType::SCALE_IN:
            return AnimationManager::getInstance().animateScaleIn(duration, callback);
        case AnimationType::SLIDE_IN_LEFT:
            return AnimationManager::getInstance().animateSlideInLeft(duration, callback);
        case AnimationType::SLIDE_IN_RIGHT:
            return AnimationManager::getInstance().animateSlideInRight(duration, callback);
        case AnimationType::BOUNCE:
            return AnimationManager::getInstance().animateBounce(duration, callback);
        case AnimationType::PULSE:
            return AnimationManager::getInstance().animatePulse(duration, true, callback);
        default:
            return AnimationManager::getInstance().animateFadeIn(duration, callback);
    }
}

void startUIAnimation(size_t animationId) {
    AnimationManager::getInstance().startAnimation(animationId);
}

void stopUIAnimation(size_t animationId) {
    AnimationManager::getInstance().stopAnimation(animationId);
}

} // namespace UIAnimation
