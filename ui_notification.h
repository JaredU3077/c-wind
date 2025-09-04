// ui_notification.h - Notification system for Browserwind
#ifndef UI_NOTIFICATION_H
#define UI_NOTIFICATION_H

#include "raylib.h"
#include "ui_theme_optimized.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace UINotification {

// ============================================================================
// NOTIFICATION TYPES AND STRUCTURES
// ============================================================================

enum class NotificationType {
    INFO,           // Blue - general information
    SUCCESS,        // Green - positive feedback
    WARNING,        // Yellow - caution/attention
    ERROR,          // Red - critical issues
    QUEST,          // Purple - quest-related
    LEVEL_UP,       // Gold - character progression
    ITEM_ACQUIRED,  // Silver - item pickups
    EXPERIENCE      // Bronze - XP gains
};

enum class NotificationPosition {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
    CENTER
};

struct NotificationData {
    std::string title;
    std::string message;
    NotificationType type;
    NotificationPosition position;
    float duration;  // seconds
    float createdTime;
    bool isPersistent;  // stays until dismissed
    std::function<void()> onClick;
    std::function<void()> onDismiss;

    // Visual properties
    Color backgroundColor;
    Color titleColor;
    Color messageColor;
    Color borderColor;
    float width;
    float height;

    NotificationData(const std::string& t = "", const std::string& msg = "",
                    NotificationType nt = NotificationType::INFO,
                    NotificationPosition pos = NotificationPosition::TOP_RIGHT,
                    float dur = 3.0f, bool persistent = false)
        : title(t), message(msg), type(nt), position(pos), duration(dur),
          createdTime(0.0f), isPersistent(persistent), width(300.0f), height(80.0f) {
        updateColors();
    }

    void updateColors() {
        switch (type) {
            case NotificationType::INFO:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::PRIMARY, 0.9f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::ACCENT);
                break;
            case NotificationType::SUCCESS:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS, 0.2f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS);
                break;
            case NotificationType::WARNING:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::WARNING, 0.2f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::WARNING);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::WARNING);
                break;
            case NotificationType::ERROR:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::ERROR, 0.2f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::ERROR);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::ERROR);
                break;
            case NotificationType::QUEST:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::EPIC, 0.2f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::EPIC);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::EPIC);
                break;
            case NotificationType::LEVEL_UP:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE, 0.2f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE);
                break;
            case NotificationType::ITEM_ACQUIRED:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::RARE, 0.2f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::RARE);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::RARE);
                break;
            case NotificationType::EXPERIENCE:
                backgroundColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE, 0.15f);
                titleColor = UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY);
                messageColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE);
                borderColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE);
                break;
        }
    }
};

class NotificationManager {
public:
    static NotificationManager& getInstance() {
        static NotificationManager instance;
        return instance;
    }

    // Notification management
    void addNotification(const NotificationData& notification);
    void addNotification(const std::string& title, const std::string& message,
                        NotificationType type = NotificationType::INFO,
                        NotificationPosition position = NotificationPosition::TOP_RIGHT,
                        float duration = 3.0f);

    void update(float deltaTime);
    void render();
    void clear();
    void dismissNotification(size_t index);

    // Quick notification methods
    void showInfo(const std::string& title, const std::string& message);
    void showSuccess(const std::string& title, const std::string& message);
    void showWarning(const std::string& title, const std::string& message);
    void showError(const std::string& title, const std::string& message);
    void showQuest(const std::string& title, const std::string& message);
    void showLevelUp(const std::string& title, const std::string& message);
    void showItemAcquired(const std::string& title, const std::string& message);
    void showExperience(const std::string& title, const std::string& message);

    // Settings
    void setMaxNotifications(int max) { maxNotifications = max; }
    void setFadeTime(float time) { fadeTime = time; }

private:
    NotificationManager() = default;
    ~NotificationManager() = default;
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;

    std::vector<NotificationData> notifications;
    int maxNotifications = 5;
    float fadeTime = 0.3f;

    Vector2 calculatePosition(const NotificationData& notification, int index);
    void renderNotification(const NotificationData& notification, Vector2 position, float alpha);
};

// ============================================================================
// GLOBAL FUNCTIONS FOR EASY ACCESS
// ============================================================================

void showNotification(const std::string& title, const std::string& message,
                     NotificationType type = NotificationType::INFO,
                     NotificationPosition position = NotificationPosition::TOP_RIGHT,
                     float duration = 3.0f);

void showInfoNotification(const std::string& title, const std::string& message);
void showSuccessNotification(const std::string& title, const std::string& message);
void showWarningNotification(const std::string& title, const std::string& message);
void showErrorNotification(const std::string& title, const std::string& message);
void showQuestNotification(const std::string& title, const std::string& message);
void showLevelUpNotification(const std::string& title, const std::string& message);
void showItemNotification(const std::string& title, const std::string& message);
void showXPNotification(const std::string& title, const std::string& message);

} // namespace UINotification

#endif // UI_NOTIFICATION_H
