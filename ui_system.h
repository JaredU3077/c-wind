// ui_system.h - Browserwind UI System with integrated Design System
#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include "raylib.h"
#include "game_state.h"
#include "inventory.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>

// ============================================================================
// BROWSERWIND UI DESIGN SYSTEM
// ============================================================================
// Unified fantasy-themed UI system for Browserwind
// Uses optimized theme system for consistency and performance

#include "ui_theme_optimized.h"

namespace UIDesign {

// ============================================================================
// LEGACY COMPATIBILITY CONSTANTS
// ============================================================================
// These constants are maintained for backward compatibility but now
// delegate to the optimized theme system for consistency

// Primary Colors - Delegated to theme system
inline Color getPrimaryDark()     { return UITypes::GetThemeColor(UITypes::ColorRole::PRIMARY); }
inline Color getPrimaryMedium()   { return UITypes::GetThemeColor(UITypes::ColorRole::PRIMARY); }
inline Color getPrimaryLight()    { return UITypes::GetThemeColor(UITypes::ColorRole::PRIMARY); }
inline Color getPrimaryAccent()   { return UITypes::GetThemeColor(UITypes::ColorRole::ACCENT); }

// Secondary Colors - Delegated to theme system
inline Color getSecondaryDark()   { return UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY); }
inline Color getSecondaryMedium() { return UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY); }
inline Color getSecondaryLight()  { return UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY); }
inline Color getSecondaryAccent() { return UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY); }

// Status Colors - Delegated to theme system
inline Color getHealthColor()     { return UITypes::GetThemeColor(UITypes::ColorRole::HEALTH); }
inline Color getManaColor()       { return UITypes::GetThemeColor(UITypes::ColorRole::MANA); }
inline Color getStaminaColor()    { return UITypes::GetThemeColor(UITypes::ColorRole::STAMINA); }
inline Color getExperienceColor() { return UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE); }

// Rarity Colors - Delegated to theme system
inline Color getRarityCommon()    { return UITypes::GetThemeColor(UITypes::ColorRole::COMMON); }
inline Color getRarityUncommon()  { return UITypes::GetThemeColor(UITypes::ColorRole::UNCOMMON); }
inline Color getRarityRare()      { return UITypes::GetThemeColor(UITypes::ColorRole::RARE); }
inline Color getRarityEpic()      { return UITypes::GetThemeColor(UITypes::ColorRole::EPIC); }
inline Color getRarityLegendary() { return UITypes::GetThemeColor(UITypes::ColorRole::LEGENDARY); }
inline Color getRarityArtifact()  { return UITypes::GetThemeColor(UITypes::ColorRole::ARTIFACT); }

// UI State Colors - Delegated to theme system
inline Color getTextNormal()      { return UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY); }
inline Color getTextHighlight()   { return UITypes::GetThemeColor(UITypes::ColorRole::ACCENT); }
inline Color getTextDisabled()    { return UITypes::GetThemeColor(UITypes::ColorRole::TEXT_DISABLED); }
inline Color getTextSubtle()      { return UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY); }
inline Color getTextWarning()     { return UITypes::GetThemeColor(UITypes::ColorRole::WARNING); }
inline Color getTextError()       { return UITypes::GetThemeColor(UITypes::ColorRole::ERROR); }
inline Color getTextSuccess()     { return UITypes::GetThemeColor(UITypes::ColorRole::SUCCESS); }

// Interactive States - Delegated to theme system
inline Color getHoverBackground()     { return UITypes::GetThemeColor(UITypes::ColorRole::HOVER); }
inline Color getSelectedBackground()  { return UITypes::GetThemeColor(UITypes::ColorRole::SELECTED); }
inline Color getPressedBackground()   { return UITypes::GetThemeColor(UITypes::ColorRole::PRESSED); }

// Font access - Delegated to theme system
inline const Font* getUIFont()        { return UITypes::GetThemeFont(UITypes::FontRole::UI); }
inline const Font* getHeaderFont()    { return UITypes::GetThemeFont(UITypes::FontRole::HEADER); }
inline const Font* getBodyFont()      { return UITypes::GetThemeFont(UITypes::FontRole::BODY); }

// Background colors - Delegated to theme system
inline Color getBackgroundColor()     { return UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND); }
inline Color getSurfaceColor()        { return UITypes::GetThemeColor(UITypes::ColorRole::SURFACE); }

// Text colors - Delegated to theme system
inline Color getTextPrimary()         { return UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY); }
inline Color getTextSecondary()       { return UITypes::GetThemeColor(UITypes::ColorRole::TEXT_SECONDARY); }

// ============================================================================
// TYPOGRAPHY SYSTEM - DELEGATED TO THEME
// ============================================================================

struct FontStyle {
    int size;
    Color color;
    bool bold;
    float spacing;
};

// Standard font styles - now using theme system
inline FontStyle getFontTitle() {
    return {24, UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY), true, 1.0f};
}

inline FontStyle getFontHeader() {
    return {18, UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY), false, 1.0f};
}

inline FontStyle getFontBody() {
    return {14, UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY), false, 1.0f};
}

inline FontStyle getFontSmall() {
    return {12, UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY), false, 1.0f};
}

inline FontStyle getFontTiny() {
    return {10, UITypes::GetThemeColor(UITypes::ColorRole::TEXT_DISABLED), false, 1.0f};
}

// ============================================================================
// LAYOUT SYSTEM
// ============================================================================

// Spacing constants (in pixels) - now delegated to theme system
inline int getSpacingTiny()   { return static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::XS)); }
inline int getSpacingSmall()  { return static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::SM)); }
inline int getSpacingMedium() { return static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::MD)); }
inline int getSpacingLarge()  { return static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::LG)); }
inline int getSpacingXLarge() { return static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::XL)); }

// Border constants - now using theme values
inline int getBorderThin()   { return 1; }
inline int getBorderMedium() { return 2; }
inline int getBorderThick()  { return 3; }

// Corner radius for modern look - now using theme values
inline int getCornerRadius() { return 4; }

// ============================================================================
// UI COMPONENT STYLES - DELEGATED TO THEME
// ============================================================================

struct PanelStyle {
    Color backgroundColor;
    Color borderColor;
    int borderWidth;
    int cornerRadius;
    float alpha;

    static PanelStyle getDefault() {
        // TEMPORARY: Use bright colors for debugging UI visibility
        Color bgColor = {100, 150, 200, 255};  // Bright blue background
        Color borderColor = {255, 255, 255, 255};  // White border

        return {
            bgColor,
            borderColor,
            2,
            4,
            1.0f
        };
    }
};

struct ButtonStyle {
    Color backgroundColor;
    Color hoverColor;
    Color pressedColor;
    Color borderColor;
    Color textColor;
    FontStyle fontStyle;
    int borderWidth;
    int cornerRadius;
    int padding;

    static ButtonStyle getDefault() {
        return {
            UITypes::GetThemeColor(UITypes::ColorRole::SURFACE),
            UITypes::GetThemeColor(UITypes::ColorRole::HOVER),
            UITypes::GetThemeColor(UITypes::ColorRole::PRESSED),
            UITypes::GetThemeColor(UITypes::ColorRole::BORDER),
            UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY),
            UIDesign::getFontBody(),
            2,
            4,
            static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::SM))
        };
    }
};

struct ProgressBarStyle {
    Color backgroundColor;
    Color fillColor;
    Color borderColor;
    int borderWidth;
    int height;
    int cornerRadius;

    static ProgressBarStyle getDefault() {
        return {
            UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND),
            UITypes::GetThemeColor(UITypes::ColorRole::ACCENT),
            UITypes::GetThemeColor(UITypes::ColorRole::BORDER),
            2,
            static_cast<int>(UITypes::GetThemeSpacing(UITypes::SpacingRole::MD)),
            4
        };
    }
};

// Panel styles - now using inline functions for theme consistency
inline PanelStyle getPanelWindow() {
    return {
        UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND),
        UITypes::GetThemeColor(UITypes::ColorRole::ACCENT),
        getBorderMedium(),
        getCornerRadius(),
        0.95f
    };
}

inline PanelStyle getPanelPopup() {
    return {
        UITypes::GetThemeColor(UITypes::ColorRole::SURFACE),
        UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY),
        getBorderMedium(),
        getCornerRadius(),
        0.90f
    };
}

inline PanelStyle getPanelTooltip() {
    return {
        UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND),
        UITypes::GetThemeColor(UITypes::ColorRole::PRIMARY),
        getBorderThin(),
        getCornerRadius(),
        0.98f
    };
}

inline PanelStyle getPanelOverlay() {
    return {
        UITypes::GetThemeColor(UITypes::ColorRole::BACKGROUND),
        UITypes::GetThemeColor(UITypes::ColorRole::ACCENT),
        getBorderThick(),
        getCornerRadius(),
        0.85f
    };
}

// Button styles - now using inline functions for theme consistency
inline ButtonStyle getButtonPrimary() {
    return {
        UITypes::GetThemeColor(UITypes::ColorRole::PRIMARY),
        UITypes::GetThemeColor(UITypes::ColorRole::HOVER),
        UITypes::GetThemeColor(UITypes::ColorRole::PRESSED),
        UITypes::GetThemeColor(UITypes::ColorRole::ACCENT),
        UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY),
        getFontBody(),
        getBorderMedium(),
        getCornerRadius(),
        getSpacingMedium()
    };
}

inline ButtonStyle getButtonSecondary() {
    return {
        UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY),
        UITypes::GetThemeColor(UITypes::ColorRole::HOVER),
        UITypes::GetThemeColor(UITypes::ColorRole::PRESSED),
        UITypes::GetThemeColor(UITypes::ColorRole::SECONDARY),
        UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY),
        getFontBody(),
        getBorderMedium(),
        getCornerRadius(),
        getSpacingMedium()
    };
}

// Button danger style will be defined after fadeColor
// Progress bar styles will be defined after fadeColor

// ============================================================================
// DRAWING UTILITIES
// ============================================================================

// Enhanced drawing functions with consistent styling
void drawStyledPanel(const PanelStyle& style, Rectangle bounds);
void drawStyledButton(const ButtonStyle& style, Rectangle bounds, const std::string& text, bool isHovered = false, bool isPressed = false);
void drawStyledProgressBar(const ProgressBarStyle& style, Rectangle bounds, float progress, const std::string& label = "");
void drawStyledText(const std::string& text, Vector2 position, const FontStyle& style);
void drawStyledTooltip(const std::string& text, Vector2 position, int maxWidth = 250);

// Fantasy-themed decorative elements
void drawOrnateBorder(Rectangle bounds, Color color, int thickness = 2);
void drawRuneDecoration(Vector2 position, float size, Color color);
void drawScrollDecoration(Rectangle bounds, Color color);

// Helper functions
Rectangle centerRectInRect(Rectangle inner, Rectangle outer);
Rectangle positionRectRelative(Rectangle base, int offsetX, int offsetY, int width, int height);
Vector2 getTextSize(const std::string& text, const FontStyle& style);
bool isPointInRect(Vector2 point, Rectangle rect);

// Color manipulation - Delegated to theme system
inline Color fadeColor(Color color, float alpha) {
    return Color{color.r, color.g, color.b, static_cast<unsigned char>(color.a * alpha)};
}

inline Color blendColors(Color color1, Color color2, float factor) {
    return Color{
        static_cast<unsigned char>(color1.r * (1 - factor) + color2.r * factor),
        static_cast<unsigned char>(color1.g * (1 - factor) + color2.g * factor),
        static_cast<unsigned char>(color1.b * (1 - factor) + color2.b * factor),
        static_cast<unsigned char>(color1.a * (1 - factor) + color2.a * factor)
    };
}

// String formatting for UI - Delegated to theme system
inline std::string formatItemName(const std::string& name, int maxLength = 20) {
    return UITypes::FormatItemName(name, maxLength);
}

inline std::string formatValue(int value) {
    return UITypes::FormatValue(value);
}

inline std::string formatWeight(float weight) {
    return UITypes::FormatWeight(weight);
}

// Additional inline functions that depend on fadeColor - defined after fadeColor
inline ButtonStyle getButtonDanger() {
    Color dangerBg = UITypes::GetThemeColor(UITypes::ColorRole::ERROR);
    Color dangerHover = fadeColor(dangerBg, 1.2f);
    Color dangerPressed = fadeColor(dangerBg, 0.8f);
    return {
        dangerBg,
        dangerHover,
        dangerPressed,
        UITypes::GetThemeColor(UITypes::ColorRole::ERROR),
        UITypes::GetThemeColor(UITypes::ColorRole::TEXT_PRIMARY),
        getFontBody(),
        getBorderMedium(),
        getCornerRadius(),
        getSpacingMedium()
    };
}

inline ProgressBarStyle getProgressBarHealth() {
    Color healthColor = UITypes::GetThemeColor(UITypes::ColorRole::HEALTH);
    return {
        fadeColor(healthColor, 0.3f),
        healthColor,
        fadeColor(healthColor, 0.6f),
        getBorderMedium(),
        12,
        getCornerRadius()
    };
}

inline ProgressBarStyle getProgressBarMana() {
    Color manaColor = UITypes::GetThemeColor(UITypes::ColorRole::MANA);
    return {
        fadeColor(manaColor, 0.3f),
        manaColor,
        fadeColor(manaColor, 0.6f),
        getBorderMedium(),
        12,
        getCornerRadius()
    };
}

inline ProgressBarStyle getProgressBarExperience() {
    Color expColor = UITypes::GetThemeColor(UITypes::ColorRole::EXPERIENCE);
    return {
        fadeColor(expColor, 0.3f),
        expColor,
        fadeColor(expColor, 0.6f),
        getBorderMedium(),
        8,
        getCornerRadius()
    };
}

} // namespace UIDesign

// ============================================================================
// BROWSERWIND UI SYSTEM
// ============================================================================
// Main UI orchestrator for Browserwind
// Handles layout, rendering, and interaction for all UI elements

// UI Layer priorities - higher numbers render on top
enum class UILayer {
    BACKGROUND = 0,     // Ground-level UI elements (HUD, stats)
    GAMEPLAY = 1,       // Gameplay UI (crosshair, prompts, panels)
    MODAL = 2,          // Modal dialogs (NPC conversation, inventory)
    OVERLAY = 3,        // Full-screen overlays (ESC menu, loading screens)
    DEBUG = 4           // Debug/diagnostic overlays (always on top)
};

// UI positioning zones for organized layout
enum class UIZone {
    TOP_LEFT,       // Player stats, diagnostic info
    TOP_RIGHT,      // Performance stats, mini-map
    CENTER,         // Crosshair, modal dialogs
    BOTTOM_LEFT,    // Controls help, game stats
    BOTTOM_RIGHT,   // Chat, notifications
    INTERACTION     // Dynamic interaction prompts
};

// UI element configuration
struct UIElement {
    Rectangle bounds;
    UILayer layer;
    UIZone zone;
    bool visible;
    Color backgroundColor;
    Color borderColor;
    std::string title;
};

// Main UI System class
class UISystemManager {
public:
    UISystemManager();
    ~UISystemManager();

    // Core rendering system
    void renderAllUI(Camera3D camera, const GameState& state, float currentTime);

    // Layer-specific rendering functions
    void renderBackgroundLayer(const GameState& state);
    void renderGameplayLayer(Camera3D camera, const GameState& state, float currentTime);
    void renderModalLayer(const GameState& state);
    void renderOverlayLayer(GameState& state);  // Non-const for ESC menu interaction
    void renderDebugLayer(const GameState& state, float currentTime);

    // Enhanced UI rendering functions
    void renderEnhancedEscMenu(GameState& state);

    // Zone management
    Rectangle getZoneBounds(UIZone zone) const;
    bool isZoneOccupied(UIZone zone) const;
    void reserveZone(UIZone zone, const std::string& elementName);
    void releaseZone(UIZone zone);

    // Toggle panels
    void renderDetailedTestingPanel(const GameState& state, const std::string& locationText, Color locationColor);

    // Utility functions
    void setScreenDimensions(int width, int height);
    bool isModalActive(const GameState& state) const;
    void clearOverlaps();

    // Element positioning helpers
    static Rectangle positionInZone(UIZone zone, int width, int height, int offsetX = 0, int offsetY = 0);
    static bool checkUICollision(Rectangle rect1, Rectangle rect2);

private:
    int screenWidth_;
    int screenHeight_;
    std::map<UIZone, std::string> zoneReservations_;

    // Individual UI component renderers - organized and positioned
    void renderCrosshair(const GameState& state);
    void renderPlayerStats(const GameState& state);
    void renderPerformanceDisplay(const GameState& state);
    void renderControlsPanel(const GameState& state);
    void renderGameStats(const GameState& state);
    void renderInteractionPrompt(const GameState& state, float currentTime);
    void renderDiagnosticPanel(const GameState& state);
    void renderTestingStatus(const GameState& state);

    // Modal/overlay renderers
    void renderInventoryModal(const GameState& state);
    void renderDialogModal(const GameState& state);
    void renderEscMenuOverlay(GameState& state);

    // Layout helpers
    void initializeLayout();
    Rectangle calculateNonOverlappingPosition(UIZone preferredZone, int width, int height) const;
};

// Global UI system instance
extern UISystemManager* g_uiSystem;

// Global inventory component instance
// Forward declaration for InventoryUIComponent
class InventoryUIComponent;
extern InventoryUIComponent* g_inventoryComponent;

// Initialization function for main.cpp
void initializeUISystem();
void shutdownUISystem();

#endif // UI_SYSTEM_H
