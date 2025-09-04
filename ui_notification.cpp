// ui_notification.cpp - Implementation of the notification system
#include "ui_notification.h"
#include <algorithm>
#include <iostream>

namespace UINotification {

// ============================================================================
// NOTIFICATION MANAGER IMPLEMENTATION
// ============================================================================

void NotificationManager::addNotification(const NotificationData& notification) {
    if (notifications.size() >= static_cast<size_t>(maxNotifications)) {
        notifications.erase(notifications.begin()); // Remove oldest
    }

    NotificationData newNotification = notification;
    newNotification.createdTime = GetTime();
    newNotification.updateColors();
    notifications.push_back(newNotification);
}

void NotificationManager::addNotification(const std::string& title, const std::string& message,
                                        NotificationType type, NotificationPosition position,
                                        float duration) {
    NotificationData notification(title, message, type, position, duration);
    addNotification(notification);
}

void NotificationManager::update([[maybe_unused]] float deltaTime) {
    float currentTime = GetTime();

    // Remove expired notifications
    notifications.erase(
        std::remove_if(notifications.begin(), notifications.end(),
            [currentTime](const NotificationData& notification) {
                return !notification.isPersistent &&
                       (currentTime - notification.createdTime) > notification.duration;
            }),
        notifications.end()
    );
}

void NotificationManager::render() {
    float currentTime = GetTime();

    for (size_t i = 0; i < notifications.size(); ++i) {
        const auto& notification = notifications[i];
        Vector2 position = calculatePosition(notification, i);

        // Calculate fade alpha
        float elapsedTime = currentTime - notification.createdTime;
        float alpha = 1.0f;

        if (elapsedTime < fadeTime) {
            // Fade in
            alpha = elapsedTime / fadeTime;
        } else if (!notification.isPersistent &&
                   elapsedTime > (notification.duration - fadeTime)) {
            // Fade out
            float remainingTime = notification.duration - elapsedTime;
            alpha = remainingTime / fadeTime;
        }

        alpha = std::max(0.0f, std::min(1.0f, alpha));
        renderNotification(notification, position, alpha);
    }
}

void NotificationManager::clear() {
    notifications.clear();
}

void NotificationManager::dismissNotification(size_t index) {
    if (index < notifications.size()) {
        if (notifications[index].onDismiss) {
            notifications[index].onDismiss();
        }
        notifications.erase(notifications.begin() + index);
    }
}

// Quick notification methods
void NotificationManager::showInfo(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::INFO);
}

void NotificationManager::showSuccess(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::SUCCESS);
}

void NotificationManager::showWarning(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::WARNING);
}

void NotificationManager::showError(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::ERROR);
}

void NotificationManager::showQuest(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::QUEST);
}

void NotificationManager::showLevelUp(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::LEVEL_UP);
}

void NotificationManager::showItemAcquired(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::ITEM_ACQUIRED);
}

void NotificationManager::showExperience(const std::string& title, const std::string& message) {
    addNotification(title, message, NotificationType::EXPERIENCE);
}

Vector2 NotificationManager::calculatePosition(const NotificationData& notification, int index) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float spacing = UITypes::GetThemeSpacing(UITypes::SpacingRole::MD);
    float totalHeight = notification.height + spacing;

    switch (notification.position) {
        case NotificationPosition::TOP_LEFT:
            return {spacing, spacing + (float)index * totalHeight};

        case NotificationPosition::TOP_CENTER:
            return {(float)screenWidth/2 - notification.width/2,
                   spacing + (float)index * totalHeight};

        case NotificationPosition::TOP_RIGHT:
            return {(float)screenWidth - notification.width - spacing,
                   spacing + (float)index * totalHeight};

        case NotificationPosition::BOTTOM_LEFT:
            return {spacing, (float)screenHeight - (float)(index + 1) * totalHeight - spacing};

        case NotificationPosition::BOTTOM_CENTER:
            return {(float)screenWidth/2 - notification.width/2,
                   (float)screenHeight - (float)(index + 1) * totalHeight - spacing};

        case NotificationPosition::BOTTOM_RIGHT:
            return {(float)screenWidth - notification.width - spacing,
                   (float)screenHeight - (float)(index + 1) * totalHeight - spacing};

        case NotificationPosition::CENTER:
            return {(float)screenWidth/2 - notification.width/2,
                   (float)screenHeight/2 - notification.height/2 + (float)index * totalHeight};

        default:
            return {spacing, spacing + (float)index * totalHeight};
    }
}

void NotificationManager::renderNotification(const NotificationData& notification,
                                           Vector2 position, float alpha) {
    // Create notification bounds
    Rectangle bounds = {
        position.x,
        position.y,
        notification.width,
        notification.height
    };

    // Apply alpha to colors
    Color bgColor = Color{notification.backgroundColor.r, notification.backgroundColor.g, notification.backgroundColor.b, (unsigned char)(notification.backgroundColor.a * alpha)};
    Color titleColor = Color{notification.titleColor.r, notification.titleColor.g, notification.titleColor.b, (unsigned char)(notification.titleColor.a * alpha)};
    Color messageColor = Color{notification.messageColor.r, notification.messageColor.g, notification.messageColor.b, (unsigned char)(notification.messageColor.a * alpha)};
    Color borderColor = Color{notification.borderColor.r, notification.borderColor.g, notification.borderColor.b, (unsigned char)(notification.borderColor.a * alpha)};

    // Draw background with theme
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 2.0f, borderColor);

    // Draw title
    if (!notification.title.empty()) {
        Vector2 titlePos = {
            bounds.x + UITypes::GetThemeSpacing(UITypes::SpacingRole::MD),
            bounds.y + UITypes::GetThemeSpacing(UITypes::SpacingRole::SM)
        };
        DrawText(notification.title.c_str(), (int)titlePos.x, (int)titlePos.y, 18, titleColor);
    }

    // Draw message
    if (!notification.message.empty()) {
        Vector2 messagePos = {
            bounds.x + UITypes::GetThemeSpacing(UITypes::SpacingRole::MD),
            bounds.y + UITypes::GetThemeSpacing(UITypes::SpacingRole::MD) + 18
        };
        DrawText(notification.message.c_str(), (int)messagePos.x, (int)messagePos.y, 14, messageColor);
    }

    // Add a subtle glow effect for important notifications
    if (notification.type == NotificationType::LEVEL_UP ||
        notification.type == NotificationType::QUEST) {
        Rectangle glowBounds = {
            bounds.x - 2,
            bounds.y - 2,
            bounds.width + 4,
            bounds.height + 4
        };
        Color glowColor = Color{notification.borderColor.r, notification.borderColor.g, notification.borderColor.b, (unsigned char)(notification.borderColor.a * alpha * 0.3f)};
        DrawRectangleRec(glowBounds, glowColor);
    }
}

// ============================================================================
// GLOBAL FUNCTIONS FOR EASY ACCESS
// ============================================================================

void showNotification(const std::string& title, const std::string& message,
                     NotificationType type, NotificationPosition position,
                     float duration) {
    NotificationManager::getInstance().addNotification(title, message, type, position, duration);
}

void showInfoNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showInfo(title, message);
}

void showSuccessNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showSuccess(title, message);
}

void showWarningNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showWarning(title, message);
}

void showErrorNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showError(title, message);
}

void showQuestNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showQuest(title, message);
}

void showLevelUpNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showLevelUp(title, message);
}

void showItemNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showItemAcquired(title, message);
}

void showXPNotification(const std::string& title, const std::string& message) {
    NotificationManager::getInstance().showExperience(title, message);
}

} // namespace UINotification
