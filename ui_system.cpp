// ui_system.cpp
#include "ui_system.h"
#include "ui_theme_optimized.h"
#include "render_utils.h"
#include "performance_system.h"
#include "debug_system.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// ============================================================================
// THEME-BASED UI DESIGN SYSTEM - Using existing UIDesign functions
// ============================================================================

namespace UIDesign {

// The existing UIDesign functions from ui_system.h are already working correctly
// with the theme system. No additional overrides needed.

} // namespace UIDesign

// ============================================================================
// UI DESIGN SYSTEM IMPLEMENTATION
// ============================================================================

namespace UIDesign {

// ============================================================================
// DRAWING UTILITIES IMPLEMENTATION
// ============================================================================

void drawStyledPanel(const PanelStyle& style, Rectangle bounds) {
    // Draw background with alpha
    Color bgColor = UIDesign::fadeColor(style.backgroundColor, style.alpha);
    DrawRectangleRec(bounds, bgColor);

    // Draw border if specified
    if (style.borderWidth > 0) {
        DrawRectangleLinesEx(bounds, style.borderWidth, style.borderColor);
    }
}

void drawStyledButton(const ButtonStyle& style, Rectangle bounds, const std::string& text, bool isHovered, bool isPressed) {
    Color bgColor = style.backgroundColor;
    if (isPressed) {
        bgColor = style.pressedColor;
    } else if (isHovered) {
        bgColor = style.hoverColor;
    }

    // Draw button background
    DrawRectangleRec(bounds, bgColor);

    // Draw border
    DrawRectangleLinesEx(bounds, style.borderWidth, style.borderColor);

    // Draw text
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text.c_str(), style.fontStyle.size, style.fontStyle.spacing);
    Vector2 textPos = {
        bounds.x + (bounds.width - textSize.x) / 2,
        bounds.y + (bounds.height - textSize.y) / 2
    };
    DrawText(text.c_str(), textPos.x, textPos.y, style.fontStyle.size, style.textColor);
}

void drawStyledProgressBar(const ProgressBarStyle& style, Rectangle bounds, float progress, const std::string& label) {
    // Clamp progress between 0 and 1
    progress = std::max(0.0f, std::min(1.0f, progress));

    // Draw background
    DrawRectangleRec(bounds, style.backgroundColor);

    // Draw fill
    Rectangle fillBounds = {
        bounds.x + style.borderWidth,
        bounds.y + style.borderWidth,
        (bounds.width - 2 * style.borderWidth) * progress,
        bounds.height - 2 * style.borderWidth
    };
    DrawRectangleRec(fillBounds, style.fillColor);

    // Draw border
    DrawRectangleLinesEx(bounds, style.borderWidth, style.borderColor);

    // Draw label if provided
    if (!label.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), label.c_str(), 10, 1.0f);
        Vector2 textPos = {
            bounds.x + (bounds.width - textSize.x) / 2,
            bounds.y + (bounds.height - textSize.y) / 2
        };
        DrawText(label.c_str(), textPos.x, textPos.y, 10, UIDesign::getTextNormal());
    }
}

void drawStyledText(const std::string& text, Vector2 position, const FontStyle& style) {
    // Use DrawTextEx with proper font support
    Font* font = nullptr;
    if (style.size > 16) {
        font = const_cast<Font*>(UITypes::GetThemeFont(UITypes::FontRole::HEADER));
    } else if (style.size > 12) {
        font = const_cast<Font*>(UITypes::GetThemeFont(UITypes::FontRole::BODY));
    } else {
        font = const_cast<Font*>(UITypes::GetThemeFont(UITypes::FontRole::UI));
    }

    if (font && font->texture.id != 0) {
        DrawTextEx(*font, text.c_str(), position, style.size, style.spacing, style.color);
    } else {
        // Fallback to default font
        DrawText(text.c_str(), (int)position.x, (int)position.y, style.size, style.color);
    }
}

void drawStyledTooltip(const std::string& text, Vector2 position, int maxWidth) {
    // Simple tooltip implementation - could be enhanced
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text.c_str(), UIDesign::getFontSmall().size, 1.0f);
    Rectangle tooltipBounds = {
        position.x,
        position.y - textSize.y - 5,
        std::min((float)maxWidth, textSize.x + 10),
        textSize.y + 10
    };

    drawStyledPanel(UIDesign::getPanelTooltip(), tooltipBounds);
    Vector2 textPos = {tooltipBounds.x + 5, tooltipBounds.y + 5};
    drawStyledText(text, textPos, UIDesign::getFontSmall());
}

void drawOrnateBorder(Rectangle bounds, Color color, int thickness) {
    // Simple ornate border - draw multiple lines
    for (int i = 0; i < thickness; i++) {
        Rectangle borderBounds = {
            bounds.x + i,
            bounds.y + i,
            bounds.width - 2 * i,
            bounds.height - 2 * i
        };
        DrawRectangleLinesEx(borderBounds, 1, color);
    }
}

void drawRuneDecoration(Vector2 position, float size, Color color) {
    // Simple rune-like decoration - could be enhanced with actual rune shapes
    DrawCircle(position.x, position.y, size, color);
    DrawCircle(position.x, position.y, size * 0.6f, UIDesign::fadeColor(color, 0.5f));
}

void drawScrollDecoration(Rectangle bounds, Color color) {
    // Simple scroll-like decoration at bottom
    DrawRectangle(bounds.x, bounds.y + bounds.height - 5, bounds.width, 5,
                  UIDesign::fadeColor(color, 0.7f));
    DrawRectangle(bounds.x, bounds.y + bounds.height - 3, bounds.width, 3,
                  UIDesign::fadeColor(color, 0.5f));
}

// ============================================================================
// HELPER FUNCTIONS IMPLEMENTATION
// ============================================================================

Rectangle centerRectInRect(Rectangle inner, Rectangle outer) {
    return {
        outer.x + (outer.width - inner.width) / 2,
        outer.y + (outer.height - inner.height) / 2,
        inner.width,
        inner.height
    };
}

Rectangle positionRectRelative(Rectangle base, int offsetX, int offsetY, int width, int height) {
    return {
        base.x + offsetX,
        base.y + offsetY,
        (float)width,
        (float)height
    };
}

Vector2 getTextSize(const std::string& text, const FontStyle& style) {
    return MeasureTextEx(GetFontDefault(), text.c_str(), style.size, style.spacing);
}

bool isPointInRect(Vector2 point, Rectangle rect) {
    return (point.x >= rect.x && point.x <= rect.x + rect.width &&
            point.y >= rect.y && point.y <= rect.y + rect.height);
}

// Utility functions are now defined in the header file as inline functions

} // namespace UIDesign

// Global UI system instance
UISystemManager* g_uiSystem = nullptr;

void initializeUISystem() {
    if (!g_uiSystem) {
        // Initialize the UI theme system first
        bool themeLoaded = UITypes::ThemeManager::getInstance().loadTheme(UITypes::ThemeVariant::CLASSIC);
        std::cout << "UI Theme system initialized: " << (themeLoaded ? "SUCCESS" : "FAILED") << std::endl;

        // Theme system initialized successfully with proper colors

        // Initialize the main UI system
        g_uiSystem = new UISystemManager();
        std::cout << "UI System Manager initialized" << std::endl;
    }
}

void shutdownUISystem() {
    if (g_uiSystem) {
        delete g_uiSystem;
        g_uiSystem = nullptr;
    }
}

UISystemManager::UISystemManager() : screenWidth_(800), screenHeight_(600) {
    initializeLayout();
}

UISystemManager::~UISystemManager() {
    // Cleanup any resources
}

void UISystemManager::setScreenDimensions(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    initializeLayout();
}

void UISystemManager::initializeLayout() {
    // Clear existing zone reservations
    zoneReservations_.clear();
}

Rectangle UISystemManager::getZoneBounds(UIZone zone) const {
    switch (zone) {
        case UIZone::TOP_LEFT:
            return {10, 10, 250, 220};  // Increased width and height for better spacing
        case UIZone::TOP_RIGHT:
            return {(float)(screenWidth_ - 300), 10, 290, 140};  // Increased height to prevent overlap between performance and testing panels
        case UIZone::CENTER:
            return {(float)(screenWidth_ / 2 - 320), (float)(screenHeight_ / 2 - 220), 640, 440};  // Modal windows
        case UIZone::BOTTOM_LEFT:
            return {10, (float)(screenHeight_ - 180), 380, 170};  // Increased space for controls and stats
        case UIZone::BOTTOM_RIGHT:
            return {(float)(screenWidth_ - 280), (float)(screenHeight_ - 120), 270, 110};
        case UIZone::INTERACTION:
            return {(float)(screenWidth_ / 2 - 175), (float)(screenHeight_ / 2 - 25), 350, 50};  // Centered interaction prompts
        default:
            return {0, 0, 100, 100};
    }
}

bool UISystemManager::isModalActive(const GameState& state) const {
    return state.isInDialog || state.showInventoryWindow || state.showEscMenu;
}

void UISystemManager::renderAllUI(Camera3D camera, const GameState& state, float currentTime) {
    // Update screen dimensions
    setScreenDimensions(GetScreenWidth(), GetScreenHeight());

    // Clear previous zone reservations
    clearOverlaps();

    // Render in layer order (background to foreground)
    renderBackgroundLayer(state);

    if (!isModalActive(state)) {
        renderGameplayLayer(camera, state, currentTime);
    }

    renderModalLayer(state);
    renderOverlayLayer(const_cast<GameState&>(state));  // ESC menu needs to update selection
    renderDebugLayer(state, currentTime);
}

void UISystemManager::renderBackgroundLayer(const GameState& state) {
    // Always-visible core UI elements
    renderPlayerStats(state);
    renderPerformanceDisplay(state);
}

void UISystemManager::renderGameplayLayer([[maybe_unused]] Camera3D camera, const GameState& state, float currentTime) {
    // Only render when NOT in modal mode
    renderCrosshair(state);
    renderInteractionPrompt(state, currentTime);
    renderControlsPanel(state);
    renderGameStats(state);
}

void UISystemManager::renderModalLayer(const GameState& state) {
    // Modal dialogs - render one at a time, centered
    if (state.isInDialog && state.showDialogWindow) {
        renderDialogModal(state);
    } else if (state.showInventoryWindow) {
        renderInventoryModal(state);
    }
}

void UISystemManager::renderOverlayLayer(GameState& state) {
    // Full-screen overlays
    if (state.showEscMenu) {
        renderEscMenuOverlay(state);
    }
}

void UISystemManager::renderDebugLayer([[maybe_unused]] const GameState& state, [[maybe_unused]] float currentTime) {
    // Debug/diagnostic info - always on top but minimized
    renderDiagnosticPanel(state);
    
    // **TAB TOGGLE**: Show detailed testing panel when requested
    if (state.showTestingPanel) {
        // Calculate location info for testing panel
        std::string locationText = state.isInBuilding ? "INSIDE BUILDING" : "OUTSIDE";
        Color locationColor = state.isInBuilding ? UIDesign::getHealthColor() : UIDesign::getManaColor();
        renderDetailedTestingPanel(state, locationText, locationColor);
    } else {
        // Show compact testing status in top-right
        renderTestingStatus(state);
    }
}

// ===== INDIVIDUAL COMPONENT RENDERERS - PROPERLY POSITIONED =====

void UISystemManager::renderCrosshair(const GameState& state) {
    if (state.isInDialog || state.mouseReleased) return;

    int centerX = screenWidth_ / 2;
    int centerY = screenHeight_ / 2;
    Color crosshairColor = UIDesign::getTextHighlight();
    DrawLine(centerX - 10, centerY, centerX + 10, centerY, crosshairColor);
    DrawLine(centerX, centerY - 10, centerX, centerY + 10, crosshairColor);
}

void UISystemManager::renderPlayerStats(const GameState& state) {
    // **TOP-LEFT ZONE** - Enhanced with new design system
    Rectangle zone = calculateNonOverlappingPosition(UIZone::TOP_LEFT, 240, 140);
    int panelX = (int)zone.x + 5;
    int panelY = (int)zone.y + 85;

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::TOP_LEFT, "PlayerStats");

    // Use theme system panel
    Rectangle panelBounds = {(float)panelX, (float)panelY, 230.0f, 135.0f};
    UIDesign::PanelStyle panelStyle = UIDesign::PanelStyle::getDefault();
    UIDesign::drawStyledPanel(panelStyle, panelBounds);

    // Title with enhanced styling
    Rectangle titleBounds = {(float)panelX, (float)panelY, 230.0f, 25.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(),
                        UIDesign::getPrimaryLight());

    Vector2 titlePos = {(float)(panelX + UIDesign::getSpacingMedium()),
                       (float)(panelY + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Adventurer Status", titlePos, UIDesign::getFontTitle());

    int contentY = panelY + 30;

    // Health bar with enhanced styling
    float healthPercent = (float)state.playerHealth / state.maxPlayerHealth;
    Rectangle healthBarBounds = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY, 200.0f, 12.0f};

    UIDesign::ProgressBarStyle healthStyle = UIDesign::ProgressBarStyle::getDefault();
    UIDesign::drawStyledProgressBar(healthStyle, healthBarBounds, healthPercent,
                                  TextFormat("Health: %d/%d", state.playerHealth, state.maxPlayerHealth));

    contentY += 20;

    // Mana bar with enhanced styling
    float manaPercent = (float)state.playerMana / state.maxPlayerMana;
    Rectangle manaBarBounds = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY, 200.0f, 12.0f};

    UIDesign::ProgressBarStyle manaStyle = UIDesign::ProgressBarStyle::getDefault();
    UIDesign::drawStyledProgressBar(manaStyle, manaBarBounds, manaPercent,
                                  TextFormat("Mana: %d/%d", state.playerMana, state.maxPlayerMana));

    contentY += 20;

    // Level and experience with enhanced styling
    Vector2 levelPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(TextFormat("Level %d | XP: %d", state.playerLevel, state.playerExperience),
                               levelPos, UIDesign::getFontBody());

    contentY += 18;

    // Experience bar
    float xpPercent = state.playerLevel > 0 ? (float)state.playerExperience / (state.playerLevel * 100) : 0.0f;
    Rectangle xpBarBounds = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY, 200.0f, 8.0f};
    UIDesign::ProgressBarStyle xpStyle = UIDesign::ProgressBarStyle::getDefault();
    UIDesign::drawStyledProgressBar(xpStyle, xpBarBounds, xpPercent);

    contentY += 16;

    // Equipment bonuses with enhanced styling
    if (state.inventorySystem) {
        auto totalStats = state.inventorySystem->getTotalBonuses();
        if (totalStats.damage > 0 || totalStats.armor > 0) {
            Vector2 equipPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
            UIDesign::drawStyledText(TextFormat("Equipment: +%d DMG, +%d ARM",
                                                  totalStats.damage, totalStats.armor),
                                       equipPos, UIDesign::getFontSmall());
        }
    }

    // Add ID tag
    Vector2 idPos = {(float)(panelX + panelBounds.width - 25), (float)(panelBounds.y + 2)};
    UIDesign::drawStyledText("[ID:1]", idPos, UIDesign::getFontTiny());

    // Add subtle decorative elements
    UIDesign::drawRuneDecoration({(float)(panelX + 10), (float)(panelY + 10)}, 8.0f,
                               UIDesign::getPrimaryLight());
    UIDesign::drawRuneDecoration({(float)(panelX + 220), (float)(panelY + 10)}, 8.0f,
                               UIDesign::getPrimaryLight());
}

void UISystemManager::renderPerformanceDisplay([[maybe_unused]] const GameState& state) {
    // **TOP-RIGHT ZONE** - Enhanced performance display with new design system
    Rectangle zone = getZoneBounds(UIZone::TOP_RIGHT);
    Rectangle panelBounds = {(float)(zone.x + zone.width - 140), (float)(zone.y + 10), 130.0f, 80.0f};

    // Use new design system panel
    UIDesign::drawStyledPanel(UIDesign::getPanelPopup(), panelBounds);

    // Title with enhanced styling
    Rectangle titleBounds = {(float)(panelBounds.x), (float)(panelBounds.y), (float)(panelBounds.width), 18.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getExperienceColor(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getSecondaryLight());

    Vector2 titlePos = {(float)(panelBounds.x + UIDesign::getSpacingMedium()), (float)(panelBounds.y + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Performance", titlePos, UIDesign::getFontSmall());

    // FPS display with enhanced styling
    int fpsX = (int)panelBounds.x + UIDesign::getSpacingMedium();
    int fpsY = (int)panelBounds.y + 25;

    // Get current FPS
    int fps = GetFPS();
    Color fpsColor = fps >= 55 ? UIDesign::getTextHighlight() :
                    (fps >= 30 ? UIDesign::getTextWarning() : UIDesign::getTextError());

    Vector2 fpsPos = {(float)fpsX, (float)fpsY};
    std::string fpsText = "FPS: " + std::to_string(fps);
    UIDesign::drawStyledText(fpsText, fpsPos, {UIDesign::getFontSmall().size, fpsColor, false, 1.0f});

    // Frame time display
    Vector2 frameTimePos = {(float)fpsX, (float)(fpsY + 16)};
    float frameTime = GetFrameTime() * 1000.0f; // Convert to milliseconds
    std::string frameTimeText = TextFormat("Frame: %.1fms", frameTime);
    UIDesign::drawStyledText(frameTimeText, frameTimePos, UIDesign::getFontTiny());

    // Performance status indicator
    Vector2 statusPos = {(float)fpsX, (float)(fpsY + 30)};
    std::string statusText = fps >= 55 ? "Status: Excellent" :
                            (fps >= 30 ? "Status: Good" : "Status: Poor");
    Color statusColor = fps >= 55 ? UIDesign::getTextHighlight() :
                     (fps >= 30 ? UIDesign::getTextWarning() : UIDesign::getTextError());
    UIDesign::drawStyledText(statusText, statusPos, {UIDesign::getFontTiny().size, statusColor, false, 1.0f});

    // Add ID tag
    Vector2 idPos = {(float)(panelBounds.x + panelBounds.width - 25), (float)(panelBounds.y + 2)};
    UIDesign::drawStyledText("[ID:12]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add decorative element
    UIDesign::drawRuneDecoration({(float)(panelBounds.x + 10), (float)(panelBounds.y + 65)}, 6.0f, UIDesign::getExperienceColor());
}

void UISystemManager::renderControlsPanel([[maybe_unused]] const GameState& state) {
    // **BOTTOM-LEFT ZONE** - Enhanced controls panel with new design system
    Rectangle zoneBounds = getZoneBounds(UIZone::BOTTOM_LEFT);
    Rectangle zone = calculateNonOverlappingPosition(UIZone::BOTTOM_LEFT, zoneBounds.width - 10, 65);
    int panelX = (int)zone.x + 5;
    int panelY = (int)zone.y + 5;

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::BOTTOM_LEFT, "ControlsPanel");

    // Use new design system panel
    Rectangle panelBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 60.0f};
    UIDesign::drawStyledPanel(UIDesign::getPanelPopup(), panelBounds);

    // Title with enhanced styling
    Rectangle titleBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 20.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getSecondaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getSecondaryLight());

    Vector2 titlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Controls", titlePos, UIDesign::getFontBody());

    int contentY = panelY + 25;

    // Movement controls
    Vector2 movePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText("WASD: Move | Mouse: Look | Space: Jump", movePos, UIDesign::getFontSmall());

    contentY += 14;

    // Action controls
    Vector2 actionPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText("LMB: Attack | E: Interact | I: Inventory", actionPos, UIDesign::getFontSmall());

    contentY += 14;

    // System controls
    Vector2 systemPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText("ESC: Menu | TAB: Testing Panel", systemPos, UIDesign::getFontTiny());

    // Add ID tag
    Vector2 idPos = {(float)(panelBounds.x + panelBounds.width - 25), (float)(panelBounds.y + 2)};
    UIDesign::drawStyledText("[ID:4]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add decorative element
    UIDesign::drawRuneDecoration({(float)(panelX + 10), (float)(panelY + 45)}, 6.0f, UIDesign::getSecondaryLight());
}

void UISystemManager::renderGameStats(const GameState& state) {
    // **BOTTOM-LEFT ZONE** - Enhanced game stats with new design system
    Rectangle zoneBounds = getZoneBounds(UIZone::BOTTOM_LEFT);
    Rectangle zone = calculateNonOverlappingPosition(UIZone::BOTTOM_LEFT, zoneBounds.width - 10, 100);
    int panelX = (int)zone.x + 5;
    int panelY = (int)zone.y + 70;

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::BOTTOM_LEFT, "GameStats");

    // Use new design system panel
    Rectangle panelBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 95.0f};
    UIDesign::drawStyledPanel(UIDesign::getPanelPopup(), panelBounds);

    // Title with enhanced styling
    Rectangle titleBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 20.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getExperienceColor(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getSecondaryLight());

    Vector2 titlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Adventure Stats", titlePos, UIDesign::getFontBody());

    int contentY = panelY + 25;

    // Score with enhanced styling
    Vector2 scorePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(TextFormat("Score: %d points", state.score), scorePos, UIDesign::getFontSmall());

    contentY += 16;

    // Combat stats
    Vector2 combatPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(TextFormat("Attacks: %d", state.swingsPerformed), combatPos, UIDesign::getFontSmall());

    contentY += 14;

    Vector2 hitsPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(TextFormat("Hits: %d", state.meleeHits), hitsPos, UIDesign::getFontSmall());

    contentY += 14;

    // Accuracy with color coding
    float accuracy = state.swingsPerformed > 0 ? (float)state.meleeHits / state.swingsPerformed * 100.0f : 0.0f;
    Color accuracyColor = accuracy >= 80.0f ? UIDesign::getTextHighlight() :
                         (accuracy >= 50.0f ? UIDesign::getTextWarning() : UIDesign::getTextError());

    Vector2 accuracyPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    std::string accuracyText = TextFormat("Accuracy: %.1f%%", accuracy);
    UIDesign::drawStyledText(accuracyText, accuracyPos, {UIDesign::getFontSmall().size, accuracyColor, false, 1.0f});

    contentY += 14;

    // Combat effectiveness indicator
    std::string effectiveness = accuracy >= 80.0f ? "Excellent" :
                               (accuracy >= 60.0f ? "Good" :
                               (accuracy >= 40.0f ? "Fair" : "Poor"));
    Vector2 effectPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(TextFormat("Combat: %s", effectiveness.c_str()), effectPos, UIDesign::getFontTiny());

    // Add ID tag
    Vector2 idPos = {(float)(panelBounds.x + panelBounds.width - 25), (float)(panelBounds.y + 2)};
    UIDesign::drawStyledText("[ID:5]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add decorative element
    UIDesign::drawRuneDecoration({(float)(panelX + panelBounds.width - 20), (float)(panelY + 75)}, 8.0f, UIDesign::getExperienceColor());
}

void UISystemManager::renderInteractionPrompt(const GameState& state, float currentTime) {
    if (!state.showInteractPrompt || state.isInDialog) return;

    // **INTERACTION ZONE** - Enhanced interaction prompt with new design system
    Rectangle zone = getZoneBounds(UIZone::INTERACTION);

    // Create pulsing animation
    float pulseAlpha = 0.85f + sinf(currentTime * 4.0f) * 0.1f;

    // Use new design system panel with pulsing
    Rectangle promptBounds = {(float)zone.x, (float)zone.y, 350.0f, 45.0f};
    UIDesign::PanelStyle promptStyle = UIDesign::getPanelPopup();
    promptStyle.alpha = pulseAlpha;

    UIDesign::drawStyledPanel(promptStyle, promptBounds);

    // Main prompt text with enhanced styling
    Vector2 promptPos = {(float)(zone.x + UIDesign::getSpacingMedium()), (float)(zone.y + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText(state.interactPromptText, promptPos, UIDesign::getFontBody());

    // Action hint
    Vector2 actionPos = {(float)(zone.x + UIDesign::getSpacingMedium()), (float)(zone.y + 22)};
    UIDesign::drawStyledText("[E] to interact", actionPos, UIDesign::getFontSmall());

    // Interactive indicator with pulsing
    float indicatorAlpha = 0.7f + sinf(currentTime * 6.0f) * 0.3f;
    Color indicatorColor = UIDesign::fadeColor(UIDesign::getTextWarning(), indicatorAlpha);
    DrawCircle((float)zone.x + 330.0f, (float)zone.y + 22.0f, 4.0f, indicatorColor);

    // Add ID tag
    Vector2 idPos = {(float)(zone.x + 330), (float)(zone.y + 2)};
    UIDesign::drawStyledText("[ID:11]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add decorative element
    UIDesign::drawRuneDecoration({(float)(zone.x + 320), (float)(zone.y + 8)}, 6.0f, UIDesign::getTextWarning());
}

void UISystemManager::renderDiagnosticPanel(const GameState& state) {
    // **TOP-LEFT ZONE** - Enhanced diagnostic panel with new design system
    Rectangle zone = getZoneBounds(UIZone::TOP_LEFT);
    int panelX = (int)zone.x + 5;
    int panelY = (int)zone.y + 5;

    // **TITLE PANEL** - Browserwind title with enhanced styling
    Rectangle titleBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 30.0f};
    UIDesign::drawStyledPanel(UIDesign::getPanelWindow(), titleBounds);

    Vector2 titlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Browserwind", titlePos, UIDesign::getFontTitle());

    // Add decorative elements to title
    UIDesign::drawRuneDecoration({(float)(panelX + 10), (float)(panelY + 8)}, 8.0f, UIDesign::getPrimaryLight());
    UIDesign::drawRuneDecoration({(float)(panelX + zone.width - 20), (float)(panelY + 8)}, 8.0f, UIDesign::getPrimaryLight());

    // **GAME STATE PANEL** - Below title with enhanced styling
    Rectangle stateBounds = {(float)panelX, (float)(panelY + 40), (float)(zone.width - 10), 50.0f};
    UIDesign::drawStyledPanel(UIDesign::getPanelPopup(), stateBounds);

    // State title
    Vector2 stateTitlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + 45)};
    UIDesign::drawStyledText("System Status", stateTitlePos, UIDesign::getFontBody());

    int contentY = panelY + 65;

    // Mouse state with enhanced styling
    Color mouseStateColor = state.mouseReleased ? UIDesign::getTextWarning() : UIDesign::getTextHighlight();
    const char* mouseStateText = state.mouseReleased ? "Mouse: FREE" : "Mouse: CAPTURED";
    Vector2 mousePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(mouseStateText, mousePos, {UIDesign::getFontSmall().size, mouseStateColor, false, 1.0f});

    contentY += 15;

    // Game state with enhanced styling
    Color gameStateColor;
    std::string gameStateText;

    if (state.showEscMenu) {
        gameStateColor = UIDesign::getTextError();
        gameStateText = "Game: PAUSED (ESC Menu)";
    } else if (state.showInventoryWindow) {
        gameStateColor = UIDesign::getTextWarning();
        gameStateText = "Game: PAUSED (Inventory)";
    } else if (state.isInDialog) {
        gameStateColor = UIDesign::getTextWarning();
        gameStateText = "Game: DIALOG ACTIVE";
    } else {
        gameStateColor = UIDesign::getTextHighlight();
        gameStateText = "Game: ACTIVE";
    }

    Vector2 gameStatePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText(gameStateText, gameStatePos, {UIDesign::getFontSmall().size, gameStateColor, false, 1.0f});

    // Add status indicator
    Color statusIndicatorColor = (state.showEscMenu || state.showInventoryWindow || state.isInDialog) ?
                                UIDesign::getTextError() : UIDesign::getTextHighlight();
    DrawCircle((float)(panelX + zone.width - 25), (float)(contentY + 6), 4.0f, statusIndicatorColor);

    // Add ID tag
    Vector2 idPos = {(float)(panelX + zone.width - 25), (float)(panelY + 2)};
    UIDesign::drawStyledText("[ID:10]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});
}

void UISystemManager::renderTestingStatus(const GameState& state) {
    // **TOP-RIGHT ZONE** - Enhanced testing status with new design system
    Rectangle zoneBounds = getZoneBounds(UIZone::TOP_RIGHT);
    Rectangle zone = calculateNonOverlappingPosition(UIZone::TOP_RIGHT, zoneBounds.width - 10, 85);
    int panelX = (int)zone.x + 5;
    int panelY = (int)zone.y + 110;

    // Reserve this zone to prevent collisions
    reserveZone(UIZone::TOP_RIGHT, "TestingStatus");

    // Use new design system panel
    Rectangle panelBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 80.0f};
    UIDesign::drawStyledPanel(UIDesign::getPanelPopup(), panelBounds);

    // Title with enhanced styling
    Rectangle titleBounds = {(float)panelX, (float)panelY, (float)(zone.width - 10), 18.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getSecondaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getSecondaryLight());

    Vector2 titlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Testing Status", titlePos, UIDesign::getFontSmall());

    int contentY = panelY + 22;

    // Calculate progress
    int workingFeatures = (state.testWASDMovement ? 1 : 0) + (state.testSpaceJump ? 1 : 0) +
                          (state.testMouseLook ? 1 : 0) + (state.testMeleeSwing ? 1 : 0) +
                          (state.testBuildingEntry ? 1 : 0) + (state.testNPCInteraction ? 1 : 0);

    // Progress with enhanced styling and color coding
    Color progressColor = workingFeatures >= 5 ? UIDesign::getTextHighlight() :
                         (workingFeatures >= 3 ? UIDesign::getTextWarning() : UIDesign::getTextError());

    Vector2 progressPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    std::string progressText = TextFormat("Progress: %d/6 Features", workingFeatures);
    UIDesign::drawStyledText(progressText, progressPos, {UIDesign::getFontSmall().size, progressColor, false, 1.0f});

    contentY += 16;

    // Progress bar
    float progressPercent = (float)workingFeatures / 6.0f;
    Rectangle progressBarBounds = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY, (float)(zone.width - 30), 6.0f};
    UIDesign::drawStyledProgressBar(UIDesign::getProgressBarExperience(), progressBarBounds, progressPercent);

    contentY += 12;

    // Hint with enhanced styling
    Vector2 hintPos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)contentY};
    UIDesign::drawStyledText("Press TAB for details", hintPos, UIDesign::getFontTiny());

    contentY += 12;

    // Test indicators with visual status
    int indicatorY = contentY;

    // WASD indicator
    Color wasdColor = state.testWASDMovement ? UIDesign::getTextHighlight() : UIDesign::getTextDisabled();
    DrawCircle((float)(panelX + UIDesign::getSpacingMedium() + 2), (float)(indicatorY + 4), 3.0f, wasdColor);

    Vector2 wasdPos = {(float)(panelX + UIDesign::getSpacingMedium() + 10), (float)indicatorY};
    UIDesign::drawStyledText("WASD", wasdPos, {UIDesign::getFontTiny().size, wasdColor, false, 1.0f});

    // Mouse indicator
    Color mouseColor = state.testMouseLook ? UIDesign::getTextHighlight() : UIDesign::getTextDisabled();
    DrawCircle((float)(panelX + 70), (float)(indicatorY + 4), 3.0f, mouseColor);

    Vector2 mousePos = {(float)(panelX + 78), (float)indicatorY};
    UIDesign::drawStyledText("Mouse", mousePos, {UIDesign::getFontTiny().size, mouseColor, false, 1.0f});

    // Combat indicator
    Color combatColor = state.testMeleeSwing ? UIDesign::getTextHighlight() : UIDesign::getTextDisabled();
    DrawCircle((float)(panelX + 120), (float)(indicatorY + 4), 3.0f, combatColor);

    Vector2 combatTextPos = {(float)(panelX + 128), (float)indicatorY};
    UIDesign::drawStyledText("Combat", combatTextPos, {UIDesign::getFontTiny().size, combatColor, false, 1.0f});

    // Add ID tag
    Vector2 idPos = {(float)(panelBounds.x + panelBounds.width - 25), (float)(panelBounds.y + 2)};
    UIDesign::drawStyledText("[ID:3]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add decorative element
    UIDesign::drawRuneDecoration({(float)(panelX + panelBounds.width - 15), (float)(panelY + 65)}, 6.0f, UIDesign::getSecondaryLight());
}

// ===== MODAL RENDERERS =====

void UISystemManager::renderDialogModal(const GameState& state) {
    // **FULL-SCREEN MODAL** - Enhanced with new design system

    // **FULL-SCREEN DARK OVERLAY** - With subtle animation
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.7f));

    // **CENTERED DIALOG WINDOW** - Enhanced with new design system
    int dialogWidth = 620;
    int dialogHeight = 280;
    int dialogX = (screenWidth_ - dialogWidth) / 2;
    int dialogY = (screenHeight_ - dialogHeight) / 2;

    // Use new design system for main panel
    Rectangle dialogBounds = {(float)dialogX, (float)dialogY, (float)dialogWidth, (float)dialogHeight};
    UIDesign::drawStyledPanel(UIDesign::getPanelWindow(), dialogBounds);

    // Add ornate border decoration
    UIDesign::drawOrnateBorder(dialogBounds, UIDesign::getPrimaryAccent(), 2);

    // **DIALOG TITLE** - Enhanced with new design system
    Rectangle titleBounds = {(float)dialogX, (float)dialogY, (float)dialogWidth, 35.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getPrimaryLight());

    Vector2 titlePos = {(float)(dialogX + UIDesign::getSpacingLarge()), (float)(dialogY + UIDesign::getSpacingMedium())};
    UIDesign::drawStyledText("Conversation", titlePos, UIDesign::getFontTitle());

    // Add decorative runes to title
    UIDesign::drawRuneDecoration({(float)(dialogX + 20), (float)(dialogY + 10)}, 8.0f, UIDesign::getPrimaryLight());
    UIDesign::drawRuneDecoration({(float)(dialogX + dialogWidth - 30), (float)(dialogY + 10)}, 8.0f, UIDesign::getPrimaryLight());

    // **NPC DIALOG TEXT** - Enhanced readability
    Rectangle textAreaBounds = {(float)(dialogX + UIDesign::getSpacingLarge()), (float)(dialogY + 50),
                               (float)(dialogWidth - 2 * UIDesign::getSpacingLarge()), 60.0f};
    DrawRectangleRec(textAreaBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.3f));

    Vector2 textPos = {(float)(dialogX + UIDesign::getSpacingLarge()), (float)(dialogY + 55)};
    UIDesign::drawStyledText(state.dialogText, textPos, UIDesign::getFontBody());

    // **DIALOG OPTIONS** - Enhanced button styling with new design system
    const int buttonY = dialogY + 130;
    const int buttonHeight = 40;
    const int buttonWidth = 180;
    const int buttonSpacing = 190;
    const int startX = dialogX + UIDesign::getSpacingLarge();

    Vector2 mousePos = GetMousePosition();

    for (int i = 0; i < state.numDialogOptions; i++) {
        int buttonX = startX + (i * buttonSpacing);
        Rectangle buttonBounds = {(float)buttonX, (float)buttonY, (float)buttonWidth, (float)buttonHeight};

        bool isHovered = UIDesign::isPointInRect(mousePos, buttonBounds);

        // Use new design system button
        UIDesign::drawStyledButton(UIDesign::getButtonPrimary(), buttonBounds,
                                  state.dialogOptions[i], isHovered, false);

        // Add hover indicator
        if (isHovered) {
            DrawCircle((float)(buttonX - 12), (float)(buttonY + buttonHeight / 2), 5.0f, UIDesign::getTextWarning());
        }
    }

    // **DIALOG CONTROLS HELP** - Enhanced with new design system
    Vector2 helpPos = {(float)(dialogX + UIDesign::getSpacingLarge()), (float)(dialogY + dialogHeight - 35)};
    UIDesign::drawStyledText("Click on an option to continue the conversation", helpPos, UIDesign::getFontSmall());

    // Add ID tag
    Vector2 idPos = {(float)(dialogX + dialogWidth - 35), (float)(dialogY + 5)};
    UIDesign::drawStyledText("[ID:6]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add bottom decorative element
    UIDesign::drawScrollDecoration({(float)dialogX, (float)(dialogY + dialogHeight - 15), (float)dialogWidth, 15.0f}, UIDesign::getSecondaryLight());
}

// Global inventory component instance
// Old inventory component removed - using simple inventory nuclear option

void initializeInventoryComponent() {
    // STUB: Old inventory system disabled - using simple inventory nuclear option
    std::cout << "initializeInventoryComponent() - STUB (using simple inventory instead)" << std::endl;
}

void shutdownInventoryComponent() {
    // STUB: Old inventory system disabled
    std::cout << "shutdownInventoryComponent() - STUB" << std::endl;
}

void updateInventoryComponent([[maybe_unused]] float deltaTime, [[maybe_unused]] const GameState& state) {
    // STUB: Old inventory system disabled
}

void renderInventoryComponent([[maybe_unused]] const GameState& state) {
    // STUB: Old inventory system disabled
}

void handleInventoryComponentInput([[maybe_unused]] const GameState& state) {
    // STUB: Old inventory system disabled
}

void showInventoryComponent() {
    // STUB: Old inventory system disabled
}

void hideInventoryComponent() {
    // STUB: Old inventory system disabled
}

void UISystemManager::renderInventoryModal(const GameState& state) {
    std::cout << "DEBUG: Rendering inventory modal through UI system" << std::endl;

    // **CRITICAL SAFETY CHECKS** - Prevent crashes
    if (!state.inventorySystem) {
        std::cout << "INVENTORY ERROR: Inventory system not initialized!" << std::endl;
        // Draw a simple error message instead of crashing
        DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.8f));
        DrawText("Inventory system not available", screenWidth_/2 - 150, screenHeight_/2, 20, RED);
        return;
    }

    // Check if theme system is available
    try {
        // Test theme system access
        Color testColor = UIDesign::getPrimaryAccent();
        if (testColor.r == 0 && testColor.g == 0 && testColor.b == 0) {
            std::cout << "UI WARNING: Theme system returning black colors - possible initialization issue" << std::endl;
        }
    } catch (...) {
        std::cout << "UI ERROR: Theme system access failed!" << std::endl;
        // Fallback to basic rendering without theme system
        DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.7f));
        DrawText("Theme system error - using fallback", screenWidth_/2 - 150, screenHeight_/2, 20, YELLOW);
        return;
    }

    // **FULL-SCREEN MODAL** - Dark overlay for focus
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.7f));

    // **CENTERED INVENTORY WINDOW** - Enhanced with new design system
    int inventoryWidth = 700;
    int inventoryHeight = 500;
    int inventoryX = (screenWidth_ - inventoryWidth) / 2;
    int inventoryY = (screenHeight_ - inventoryHeight) / 2;

    // Use new design system for main panel
    Rectangle inventoryBounds = {(float)inventoryX, (float)inventoryY, (float)inventoryWidth, (float)inventoryHeight};
    UIDesign::drawStyledPanel(UIDesign::getPanelWindow(), inventoryBounds);

    // Add ornate border decoration
    UIDesign::drawOrnateBorder(inventoryBounds, UIDesign::getPrimaryAccent(), 2);

    // **INVENTORY TITLE** - Enhanced with new design system
    Rectangle titleBounds = {(float)inventoryX, (float)inventoryY, (float)inventoryWidth, 35.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getPrimaryLight());

    Vector2 titlePos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)(inventoryY + UIDesign::getSpacingMedium())};
    UIDesign::drawStyledText("Adventurer's Inventory", titlePos, UIDesign::getFontTitle());

    // Add decorative runes to title
    UIDesign::drawRuneDecoration({(float)(inventoryX + 20), (float)(inventoryY + 10)}, 8.0f, UIDesign::getPrimaryLight());
    UIDesign::drawRuneDecoration({(float)(inventoryX + inventoryWidth - 30), (float)(inventoryY + 10)}, 8.0f, UIDesign::getPrimaryLight());

    // **INVENTORY STATS** - Weight and slots
    int contentY = inventoryY + 40;
    Rectangle statsBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY,
                           (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), 30.0f};
    DrawRectangleRec(statsBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.3f));

    auto& inventory = state.inventorySystem->getInventory();
    std::string statsText = TextFormat("Weight: %.1f/%.1f kg | Slots: %d/%d | Gold: %d",
                                     inventory.getCurrentWeight(), inventory.getMaxWeight(),
                                     inventory.getUsedSlots(), inventory.getMaxSlots(),
                                     inventory.getItemQuantity("Gold Septim"));
    Vector2 statsPos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY + UIDesign::getSpacingSmall()};
    UIDesign::drawStyledText(statsText, statsPos, UIDesign::getFontSmall());

    contentY += 35;

    // **SORTING BUTTONS** - Add sorting functionality
    Vector2 sortTitlePos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY};
    UIDesign::drawStyledText("Sort by:", sortTitlePos, UIDesign::getFontSmall());
    contentY += 20;

    // Sort buttons row
    const char* sortOptions[] = {"Name", "Type", "Value", "Weight", "Rarity"};
    Vector2 mousePos = GetMousePosition();
    const int buttonWidth = 80;
    const int buttonHeight = 25;
    const int buttonSpacing = 85;
    int startX = inventoryX + UIDesign::getSpacingLarge();

    for (int i = 0; i < 5; i++) {
        int buttonX = startX + (i * buttonSpacing);
        Rectangle buttonBounds = {(float)buttonX, (float)contentY, (float)buttonWidth, (float)buttonHeight};

        bool isHovered = UIDesign::isPointInRect(mousePos, buttonBounds);
        bool isPressed = isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // Handle sort button clicks
        if (isPressed) {
            switch (i) {
                case 0: inventory.sortByName(); break;
                case 1: inventory.sortByType(); break;
                case 2: inventory.sortByValue(); break;
                case 3: inventory.sortByWeight(); break;
                case 4: inventory.sortByRarity(); break;
            }
        }

        // Draw sort button
        Color buttonColor = isHovered ? UIDesign::getButtonPrimary().hoverColor :
                          UIDesign::getButtonSecondary().backgroundColor;
        DrawRectangleRec(buttonBounds, buttonColor);
        DrawRectangleLinesEx(buttonBounds, 1.0f, UIDesign::getSecondaryLight());

        Vector2 textPos = {(float)(buttonX + buttonWidth/2 - MeasureText(sortOptions[i], UIDesign::getFontSmall().size)/2),
                          (float)(contentY + buttonHeight/2 - UIDesign::getFontSmall().size/2)};
        UIDesign::drawStyledText(sortOptions[i], textPos, UIDesign::getFontSmall());
    }

    contentY += 35;

    // **SEARCH/FILTER SECTION**
    Vector2 searchTitlePos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY};
    UIDesign::drawStyledText("Search:", searchTitlePos, UIDesign::getFontSmall());
    contentY += 20;

    // Search input box
    const int searchBoxWidth = 300;
    const int searchBoxHeight = 25;
    Rectangle searchBoxBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY,
                                (float)searchBoxWidth, (float)searchBoxHeight};

    // Draw search box
    DrawRectangleRec(searchBoxBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.4f));
    DrawRectangleLinesEx(searchBoxBounds, 1.0f, UIDesign::getSecondaryLight());

    // Search placeholder text
    std::string searchText = "Type to search items...";
    Vector2 searchTextPos = {(float)(inventoryX + UIDesign::getSpacingLarge() + 5), (float)(contentY + 2)};
    UIDesign::drawStyledText(searchText, searchTextPos, UIDesign::getFontSmall());

    // Clear search button
    Rectangle clearButtonBounds = {(float)(inventoryX + UIDesign::getSpacingLarge() + searchBoxWidth + 10), (float)contentY,
                                  60.0f, (float)searchBoxHeight};
    bool clearHovered = UIDesign::isPointInRect(mousePos, clearButtonBounds);

    DrawRectangleRec(clearButtonBounds, clearHovered ? UIDesign::getButtonSecondary().hoverColor :
                       UIDesign::getButtonSecondary().backgroundColor);
    DrawRectangleLinesEx(clearButtonBounds, 1.0f, UIDesign::getSecondaryLight());

    Vector2 clearTextPos = {(float)(inventoryX + UIDesign::getSpacingLarge() + searchBoxWidth + 15),
                           (float)(contentY + searchBoxHeight/2 - UIDesign::getFontSmall().size/2)};
    UIDesign::drawStyledText("Clear", clearTextPos, UIDesign::getFontSmall());

    contentY += 35;

    // **FILTER TIPS** - Show helpful filtering information
    Vector2 filterTitlePos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY};
    UIDesign::drawStyledText("Quick Search Tips:", filterTitlePos, UIDesign::getFontSmall());
    contentY += 18;

    const char* filterTips[] = {"Try: 'sword', 'potion', 'rare', 'common', 'quest'"};
    Vector2 filterTipsPos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY};
    UIDesign::drawStyledText(filterTips[0], filterTipsPos, UIDesign::getFontTiny());
    contentY += 25;

    // **EQUIPPED ITEMS SECTION**
    Vector2 equippedTitlePos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY};
    UIDesign::drawStyledText("Equipped Items:", equippedTitlePos, UIDesign::getFontBody());
    contentY += 20;

    // Show equipped items with enhanced visualization and safety checks
    auto& equipment = state.inventorySystem->getEquipment();

    // **SAFETY CHECK** - Ensure equipment system is accessible
    try {
        // Test equipment access
        auto testItem = equipment.getEquippedItem(EquipmentSlot::MAIN_HAND);
        (void)testItem; // Avoid unused variable warning
    } catch (const std::exception& e) {
        std::cout << "INVENTORY ERROR: Equipment system access failed: " << e.what() << std::endl;
        // Continue with empty equipment display
    } catch (...) {
        std::cout << "INVENTORY ERROR: Unknown error accessing equipment system" << std::endl;
        // Continue with empty equipment display
    }
    const EquipmentSlot slotsToShow[] = {EquipmentSlot::MAIN_HAND, EquipmentSlot::OFF_HAND, EquipmentSlot::HEAD,
                                        EquipmentSlot::CHEST, EquipmentSlot::LEGS, EquipmentSlot::FEET};

    int equippedCount = 0;

    for (EquipmentSlot slot : slotsToShow) {
        if (equippedCount >= 6) break;  // Limit display to prevent overflow

        auto item = equipment.getEquippedItem(slot);
        int slotY = contentY + (equippedCount * 20);

        // Draw slot background with safety checks
        try {
            Rectangle slotBounds = {(float)(inventoryX + UIDesign::getSpacingXLarge()), (float)slotY,
                                   (float)(inventoryWidth - 2 * UIDesign::getSpacingXLarge() - 20), 18.0f};
            DrawRectangleRec(slotBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.3f));
            DrawRectangleLinesEx(slotBounds, 1.0f, UIDesign::getSecondaryLight());
        } catch (...) {
            // Fallback to basic rectangle if theme system fails
            Rectangle slotBounds = {(float)(inventoryX + 20), (float)slotY, (float)(inventoryWidth - 40), 18.0f};
            DrawRectangleRec(slotBounds, Fade(DARKGRAY, 0.3f));
            DrawRectangleLinesEx(slotBounds, 1.0f, GRAY);
        }

        // Slot icon/indicator with safety
        std::string slotIndicator = "[";
        switch (slot) {
            case EquipmentSlot::MAIN_HAND: slotIndicator += "⚔️"; break;
            case EquipmentSlot::OFF_HAND: slotIndicator += "🛡️"; break;
            case EquipmentSlot::HEAD: slotIndicator += "👑"; break;
            case EquipmentSlot::CHEST: slotIndicator += "🦺"; break;
            case EquipmentSlot::LEGS: slotIndicator += "👖"; break;
            case EquipmentSlot::FEET: slotIndicator += "👢"; break;
            default: slotIndicator += "❓"; break;
        }
        slotIndicator += "]";

        try {
            Vector2 indicatorPos = {(float)(inventoryX + UIDesign::getSpacingXLarge() + 5), (float)slotY + 1};
            UIDesign::drawStyledText(slotIndicator, indicatorPos, UIDesign::getFontSmall());
        } catch (...) {
            // Fallback to basic text if theme system fails
            DrawText(slotIndicator.c_str(), inventoryX + 25, slotY + 1, 12, WHITE);
        }

        if (item) {
            // Show equipped item with rarity color (with safety checks)
            Color itemColor = WHITE; // Default fallback
            try {
                itemColor = UIDesign::getRarityCommon();
                switch (item->getRarity()) {
                    case ItemRarity::UNCOMMON: itemColor = UIDesign::getRarityUncommon(); break;
                    case ItemRarity::RARE: itemColor = UIDesign::getRarityRare(); break;
                    case ItemRarity::EPIC: itemColor = UIDesign::getRarityEpic(); break;
                    case ItemRarity::LEGENDARY: itemColor = UIDesign::getRarityLegendary(); break;
                    case ItemRarity::ARTIFACT: itemColor = UIDesign::getRarityArtifact(); break;
                    default: break;
                }
            } catch (...) {
                // Use fallback colors if theme system fails
                switch (item->getRarity()) {
                    case ItemRarity::UNCOMMON: itemColor = GREEN; break;
                    case ItemRarity::RARE: itemColor = BLUE; break;
                    case ItemRarity::EPIC: itemColor = PURPLE; break;
                    case ItemRarity::LEGENDARY: itemColor = ORANGE; break;
                    case ItemRarity::ARTIFACT: itemColor = RED; break;
                    default: itemColor = WHITE; break;
                }
            }

            Vector2 itemPos = {(float)(inventoryX + UIDesign::getSpacingXLarge() + 30), (float)slotY + 1};
            std::string itemName = item->getDisplayName();
            if (itemName.length() > 25) {
                itemName = itemName.substr(0, 22) + "...";
            }

            // Use DrawTextEx for colored text
            DrawTextEx(GetFontDefault(), itemName.c_str(), itemPos, UIDesign::getFontSmall().size, 1.0f, itemColor);

            // Show item stats briefly
            std::string statText = "";
            if (item->getStats().damage > 0) statText += TextFormat("+%d DMG ", item->getStats().damage);
            if (item->getStats().armor > 0) statText += TextFormat("+%d ARM ", item->getStats().armor);
            if (item->getStats().health > 0) statText += TextFormat("+%d HP ", item->getStats().health);

            if (!statText.empty() && statText.length() > 30) {
                statText = statText.substr(0, 27) + "...";
            }

            if (!statText.empty()) {
                Vector2 statPos = {(float)(inventoryX + inventoryWidth - 180), (float)slotY + 1};
                UIDesign::drawStyledText(statText, statPos, UIDesign::getFontTiny());
            }

            // Durability indicator
            if (item->getMaxDurability() > 0) {
                float durabilityPercent = item->getDurability() / item->getMaxDurability();
                Color durabilityColor = durabilityPercent > 0.5f ? UIDesign::getTextHighlight() :
                                      (durabilityPercent > 0.25f ? UIDesign::getTextWarning() : UIDesign::getTextError());

                Rectangle durabilityBar = {(float)(inventoryX + inventoryWidth - 25), (float)slotY + 2, 20.0f, 3.0f};
                DrawRectangleRec(durabilityBar, UIDesign::fadeColor(durabilityColor, 0.3f));
                Rectangle durabilityFill = {(float)(inventoryX + inventoryWidth - 25), (float)slotY + 2,
                                          20.0f * durabilityPercent, 3.0f};
                DrawRectangleRec(durabilityFill, durabilityColor);
            }

            } else {
            // Empty slot
            Vector2 emptyPos = {(float)(inventoryX + UIDesign::getSpacingXLarge() + 30), (float)slotY + 1};
            UIDesign::drawStyledText("Empty", emptyPos, UIDesign::getFontTiny());
        }

        equippedCount++;
    }

    contentY += (equippedCount * 20) + 10;

    // **INVENTORY ITEMS GRID**
    Vector2 itemsTitlePos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY};
    UIDesign::drawStyledText("Inventory Items:", itemsTitlePos, UIDesign::getFontBody());
    contentY += 20;

    // Items list area
    const int itemHeight = 20;
    const int maxVisibleItems = 12;

    // Draw items background
    Rectangle itemsAreaBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)contentY,
                               (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()),
                               (float)(maxVisibleItems * itemHeight)};
    DrawRectangleRec(itemsAreaBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.2f));
    DrawRectangleLinesEx(itemsAreaBounds, 1.0f, UIDesign::getSecondaryLight());

    // Get items (filtered by search if active) with safety checks
    std::vector<std::shared_ptr<MysticalItem>> items;
    try {
        items = state.inventorySearchActive && !state.inventorySearchQuery.empty() ?
                inventory.searchItems(state.inventorySearchQuery) :
                inventory.getAllItems();
    } catch (const std::exception& e) {
        std::cout << "INVENTORY ERROR: Failed to get inventory items: " << e.what() << std::endl;
        items.clear(); // Use empty list as fallback
    } catch (...) {
        std::cout << "INVENTORY ERROR: Unknown error getting inventory items" << std::endl;
        items.clear(); // Use empty list as fallback
    }

    int displayCount = 0;

    for (size_t i = 0; i < items.size() && displayCount < maxVisibleItems; ++i) {
        auto& item = items[i];

        // **SAFETY CHECK** - Ensure item is valid
        if (!item) {
            std::cout << "INVENTORY WARNING: Null item found in inventory at index " << (int)i << std::endl;
            continue;
        }
        int itemY = contentY + (displayCount * itemHeight);

        // Check for hover state
        Rectangle itemBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)itemY,
                               (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), (float)itemHeight};
        bool isHovered = UIDesign::isPointInRect(mousePos, itemBounds);

        // Enhanced visual feedback for different states
        if (state.lastClickedItem == item->getName()) {
            // Selected item - bright highlight
            Rectangle highlightBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)itemY,
                                       (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), (float)itemHeight};
            DrawRectangleRec(highlightBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.4f));
            DrawRectangleLinesEx(highlightBounds, 2.0f, UIDesign::getPrimaryAccent());
        } else if (isHovered) {
            // Hovered item - subtle highlight
            Rectangle hoverBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)itemY,
                                    (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), (float)itemHeight};
            DrawRectangleRec(hoverBounds, UIDesign::fadeColor(UIDesign::getSecondaryAccent(), 0.2f));
            DrawRectangleLinesEx(hoverBounds, 1.0f, UIDesign::getSecondaryAccent());
        } else if (item->isEquippable() && !item->isBroken()) {
            // Equippable items get a subtle border
            Rectangle equipBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)itemY,
                                    (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), (float)itemHeight};
            DrawRectangleLinesEx(equipBounds, 1.0f, UIDesign::fadeColor(UIDesign::getTextHighlight(), 0.5f));
        }

        // Add interaction indicator for hovered items
        if (isHovered) {
            // Small indicator showing interaction is possible
            DrawCircle((float)(inventoryX + UIDesign::getSpacingLarge() + 10), (float)(itemY + itemHeight/2),
                      3.0f, UIDesign::getTextWarning());

            // Add click hint for equippable items
            if (item->isEquippable()) {
                std::string hintText = "Click to equip";
                Vector2 hintPos = {(float)(inventoryX + inventoryWidth - 120), (float)(itemY + itemHeight/2 - UIDesign::getFontTiny().size/2)};
                UIDesign::drawStyledText(hintText, hintPos, UIDesign::getFontTiny());
            }
        }

        // Item name with rarity color
        Color itemColor = UIDesign::getRarityCommon();
        switch (item->getRarity()) {
            case ItemRarity::UNCOMMON: itemColor = UIDesign::getRarityUncommon(); break;
            case ItemRarity::RARE: itemColor = UIDesign::getRarityRare(); break;
            case ItemRarity::EPIC: itemColor = UIDesign::getRarityEpic(); break;
            case ItemRarity::LEGENDARY: itemColor = UIDesign::getRarityLegendary(); break;
            case ItemRarity::ARTIFACT: itemColor = UIDesign::getRarityArtifact(); break;
            default: break;
        }

        Vector2 itemPos = {(float)(inventoryX + UIDesign::getSpacingXLarge()), (float)itemY + 2};
        std::string itemName = item->getDisplayName();
        if (itemName.length() > 30) {
            itemName = itemName.substr(0, 27) + "...";
        }

        // Use enhanced text rendering with color
        DrawTextEx(GetFontDefault(), itemName.c_str(), itemPos, UIDesign::getFontSmall().size, 1.0f, itemColor);

        // Item weight and value
        std::string itemStats = TextFormat("%.1fkg | %d gold", item->getWeight(), item->getValue());
        Vector2 statsPos = {(float)(inventoryX + inventoryWidth - 150), (float)itemY + 2};
        UIDesign::drawStyledText(itemStats, statsPos, UIDesign::getFontTiny());

        displayCount++;
    }

    // Show message if no items
    if (items.empty()) {
        Vector2 noItemsPos = {(float)(inventoryX + inventoryWidth/2 - 60), (float)(contentY + 50)};
        UIDesign::drawStyledText("No items in inventory", noItemsPos, UIDesign::getFontSmall());
    }

    // **TOOLTIP SYSTEM** - Show item details on hover
    bool showTooltip = false;
    std::string tooltipText = "";

    // Check if mouse is hovering over any item
    for (size_t i = 0; i < items.size() && i < (size_t)maxVisibleItems; ++i) {
        auto& item = items[i];
        int itemY = contentY + ((int)i * itemHeight);
        Rectangle itemBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)itemY,
                               (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), (float)itemHeight};

        if (UIDesign::isPointInRect(mousePos, itemBounds)) {
            showTooltip = true;
            tooltipText = item->getTooltip();
            break;
        }
    }

    // Show search results count if searching
    if (state.inventorySearchActive && !state.inventorySearchQuery.empty()) {
        std::string resultText = TextFormat("Search Results: %d items found", (int)items.size());
        Vector2 resultPos = {(float)(inventoryX + UIDesign::getSpacingLarge()), (float)(contentY - 25)};
        UIDesign::drawStyledText(resultText, resultPos, UIDesign::getFontSmall());
    }

    // Render tooltip if hovering
    if (showTooltip) {
        Vector2 tooltipPos = {mousePos.x + 15, mousePos.y + 15};
        UIDesign::drawStyledTooltip(tooltipText, tooltipPos, 300);
    }

    // **INVENTORY CONTROLS HELP**
    Rectangle helpBounds = {(float)(inventoryX + UIDesign::getSpacingLarge()),
                           (float)(inventoryY + inventoryHeight - 35),
                           (float)(inventoryWidth - 2 * UIDesign::getSpacingLarge()), 25.0f};
    DrawRectangleRec(helpBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.5f));

    Vector2 helpPos = {(float)(helpBounds.x + UIDesign::getSpacingMedium()), (float)(helpBounds.y + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Click items to equip/use | I or ESC to close inventory", helpPos, UIDesign::getFontSmall());

    // Add ID tag
    Vector2 idPos = {(float)(inventoryX + inventoryWidth - 35), (float)(inventoryY + 5)};
    UIDesign::drawStyledText("[ID:7]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add bottom decorative element
    UIDesign::drawScrollDecoration({(float)inventoryX, (float)(inventoryY + inventoryHeight - 15), (float)inventoryWidth, 15.0f},
                                  UIDesign::getSecondaryLight());
}

void UISystemManager::renderEscMenuOverlay(GameState& state) {
    // **FULL-SCREEN OVERLAY** - Enhanced ESC menu with new design system
    renderEnhancedEscMenu(state);
}

void UISystemManager::renderEnhancedEscMenu(GameState& state) {
    // **FULL-SCREEN OVERLAY** - Enhanced with new design system
    int screenWidth = screenWidth_;
    int screenHeight = screenHeight_;
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));

    // **CENTERED MENU WINDOW** - Enhanced with new design system
    int menuWidth = 420;
    int menuHeight = 320;
    int menuX = (screenWidth - menuWidth) / 2;
    int menuY = (screenHeight - menuHeight) / 2;

    // Use new design system for main panel
    Rectangle menuBounds = {(float)menuX, (float)menuY, (float)menuWidth, (float)menuHeight};
    UIDesign::drawStyledPanel(UIDesign::getPanelWindow(), menuBounds);

    // Add ornate border decoration
    UIDesign::drawOrnateBorder(menuBounds, UIDesign::getPrimaryAccent(), 3);

    // **TITLE** - Enhanced with new design system
    Rectangle titleBounds = {(float)menuX, (float)menuY, (float)menuWidth, 40.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getPrimaryLight());

    Vector2 titlePos = {(float)(menuX + UIDesign::getSpacingXLarge()), (float)(menuY + UIDesign::getSpacingMedium())};
    UIDesign::drawStyledText("Browserwind Paused", titlePos, UIDesign::getFontTitle());

    // Add decorative elements to title
    UIDesign::drawRuneDecoration({(float)(menuX + 30), (float)(menuY + 12)}, 10.0f, UIDesign::getPrimaryLight());
    UIDesign::drawRuneDecoration({(float)(menuX + menuWidth - 40), (float)(menuY + 12)}, 10.0f, UIDesign::getPrimaryLight());

    // **MENU OPTIONS WITH MOUSE HOVER DETECTION**
    const char* menuOptions[] = {"Resume Game", "Save Game", "Load Game", "Quit Game"};
    int optionY = menuY + 90;
    int optionSpacing = 55;
    Vector2 mousePos = GetMousePosition();

    // **DETECT HOVERED OPTION**
    int hoveredOption = -1;
    for (int i = 0; i < 4; i++) {
        Rectangle optionBounds = {(float)(menuX + UIDesign::getSpacingXLarge()), (float)(optionY + i * optionSpacing - 5),
                                 (float)(menuWidth - 2 * UIDesign::getSpacingXLarge()), 45.0f};

        if (UIDesign::isPointInRect(mousePos, optionBounds)) {
            hoveredOption = i;
            state.selectedMenuOption = i;
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        bool isSelected = (i == state.selectedMenuOption);
        Rectangle optionBounds = {(float)(menuX + UIDesign::getSpacingXLarge()), (float)(optionY + i * optionSpacing),
                                 (float)(menuWidth - 2 * UIDesign::getSpacingXLarge()), 45.0f};

        // Enhanced button styling with new design system
        Color buttonColor = UIDesign::getButtonSecondary().backgroundColor;
        if (isSelected) {
            buttonColor = UIDesign::getButtonSecondary().hoverColor;
        }

        DrawRectangleRec(optionBounds, buttonColor);
        DrawRectangleLinesEx(optionBounds, UIDesign::getBorderMedium(), UIDesign::getButtonSecondary().borderColor);

        // Add selection highlight
        if (isSelected) {
            Rectangle highlightBounds = {(float)(optionBounds.x - 3), (float)(optionBounds.y - 3),
                                       (float)(optionBounds.width + 6), (float)(optionBounds.height + 6)};
            DrawRectangleRec(highlightBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.4f));
            DrawRectangleLinesEx(highlightBounds, UIDesign::getBorderThin(), UIDesign::getPrimaryAccent());
        }

        // Option text with enhanced styling
        Vector2 textPos = {(float)(optionBounds.x + UIDesign::getSpacingMedium()),
                          (float)(optionBounds.y + UIDesign::getSpacingMedium())};
        UIDesign::drawStyledText(menuOptions[i], textPos, UIDesign::getFontBody());

        // Add hover indicator
        if (isSelected) {
            DrawCircle((float)(optionBounds.x - 15), (float)(optionBounds.y + optionBounds.height / 2),
                      6.0f, UIDesign::getTextWarning());
        }
    }

    // **DYNAMIC CONTROLS HELP**
    Rectangle helpBounds = {(float)(menuX + UIDesign::getSpacingLarge()), (float)(menuY + menuHeight - 50),
                           (float)(menuWidth - 2 * UIDesign::getSpacingLarge()), 30.0f};
    DrawRectangleRec(helpBounds, UIDesign::fadeColor(UIDesign::getSecondaryDark(), 0.5f));

    Vector2 helpPos = {(float)(helpBounds.x + UIDesign::getSpacingMedium()), (float)(helpBounds.y + UIDesign::getSpacingSmall())};

    if (hoveredOption >= 0) {
        const char* hoverHints[] = {
            "Return to your adventure!",
            "Save your current progress",
            "Load a previously saved game",
            "Exit Browserwind"
        };
        UIDesign::drawStyledText(hoverHints[hoveredOption], helpPos, UIDesign::getFontSmall());
    } else {
        UIDesign::drawStyledText("Navigate with mouse | Click to select | ESC to resume",
                               helpPos, UIDesign::getFontSmall());
    }

    // Add ID tag
    Vector2 idPos = {(float)(menuX + menuWidth - 35), (float)(menuY + 5)};
    UIDesign::drawStyledText("[ID:8]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    // Add bottom decorative element
    UIDesign::drawScrollDecoration({(float)menuX, (float)(menuY + menuHeight - 20), (float)menuWidth, 20.0f}, UIDesign::getSecondaryLight());
}

// ===== BACKGROUND LAYER HELPERS =====



Rectangle UISystemManager::positionInZone(UIZone zone, int width, int height, int offsetX, int offsetY) {
    Rectangle zoneBounds;
    switch (zone) {
        case UIZone::TOP_LEFT:
            zoneBounds = {static_cast<float>(10 + offsetX), static_cast<float>(10 + offsetY), (float)width, (float)height};
            break;
        case UIZone::TOP_RIGHT:
            zoneBounds = {static_cast<float>(800 - width - 10 + offsetX), static_cast<float>(10 + offsetY), (float)width, (float)height};  // Assuming 800px width
            break;
        case UIZone::CENTER:
            zoneBounds = {static_cast<float>(400 - width/2 + offsetX), static_cast<float>(300 - height/2 + offsetY), (float)width, (float)height};
            break;
        case UIZone::BOTTOM_LEFT:
            zoneBounds = {static_cast<float>(10 + offsetX), static_cast<float>(600 - height - 10 + offsetY), (float)width, (float)height};  // Assuming 600px height
            break;
        case UIZone::BOTTOM_RIGHT:
            zoneBounds = {static_cast<float>(800 - width - 10 + offsetX), static_cast<float>(600 - height - 10 + offsetY), (float)width, (float)height};
            break;
        case UIZone::INTERACTION:
            zoneBounds = {static_cast<float>(10 + offsetX), static_cast<float>(250 + offsetY), (float)width, (float)height};  // Mid-screen
            break;
        default:
            zoneBounds = {0, 0, (float)width, (float)height};
    }
    return zoneBounds;
}

bool UISystemManager::checkUICollision(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

void UISystemManager::reserveZone(UIZone zone, const std::string& elementName) {
    zoneReservations_[zone] = elementName;
}

void UISystemManager::releaseZone(UIZone zone) {
    zoneReservations_.erase(zone);
}

bool UISystemManager::isZoneOccupied(UIZone zone) const {
    return zoneReservations_.find(zone) != zoneReservations_.end();
}

// ===== DETAILED TESTING PANEL (TAB TOGGLE) =====

void UISystemManager::renderDetailedTestingPanel(const GameState& state, const std::string& locationText, Color locationColor) {
    // **FULL-SCREEN MODAL** - Dark overlay for focus using design system
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(UIDesign::getPrimaryDark(), 0.7f));

    // Calculate panel dimensions
    int panelWidth = screenWidth_ - 100;
    int panelHeight = screenHeight_ - 100;
    int panelX = (screenWidth_ - panelWidth) / 2;
    int panelY = (screenHeight_ - panelHeight) / 2;

    // **MAIN PANEL** using design system
    Rectangle panelBounds = {(float)panelX, (float)panelY, (float)panelWidth, (float)panelHeight};
    UIDesign::drawStyledPanel(UIDesign::getPanelWindow(), panelBounds);

    // Title with enhanced styling
    Rectangle titleBounds = {(float)panelX, (float)panelY, (float)panelWidth, 25.0f};
    DrawRectangleRec(titleBounds, UIDesign::fadeColor(UIDesign::getPrimaryAccent(), 0.9f));
    DrawRectangleLinesEx(titleBounds, UIDesign::getBorderThin(), UIDesign::getPrimaryLight());

    Vector2 titlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + UIDesign::getSpacingSmall())};
    UIDesign::drawStyledText("Browserwind - Feature Testing Checklist", titlePos, UIDesign::getFontHeader());

    // Add ID tag
    Vector2 idPos = {(float)(panelX + panelWidth - 30), (float)(panelY + 3)};
    UIDesign::drawStyledText("[ID:9]", idPos, {8, UIDesign::getTextSubtle(), false, 1.0f});

    int yOffset = panelY + 35;

    // **MOVEMENT & CONTROLS SECTION**
    Vector2 sectionTitlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)yOffset};
    UIDesign::drawStyledText("Movement & Controls", sectionTitlePos, UIDesign::getFontBody());

    // Section underline
    DrawRectangle((float)(panelX + UIDesign::getSpacingMedium()), (float)(yOffset + 18),
                  (float)(panelWidth - 2 * UIDesign::getSpacingMedium()), 2.0f,
                  UIDesign::fadeColor(UIDesign::getSecondaryAccent(), 0.6f));
    yOffset += 30;

    // Test items with status colors
    auto drawTestItem = [&](const char* label, bool tested, const char* testText, const char* untestedText) {
        Color statusColor = tested ? UIDesign::getTextHighlight() : UIDesign::getTextWarning();
        std::string text = tested ? std::string(label) + ": " + testText : std::string(label) + ": " + untestedText;
        Vector2 itemPos = {(float)(panelX + UIDesign::getSpacingLarge()), (float)yOffset};
        UIDesign::drawStyledText(text, itemPos, {UIDesign::getFontSmall().size, statusColor, false, 1.0f});
        yOffset += 18;
    };

    drawTestItem("Mouse Capture", state.testMouseCaptured, "WORKING", "BROKEN");
    drawTestItem("WASD Movement", state.testWASDMovement, "TESTED", "TEST W/A/S/D KEYS");
    drawTestItem("Space Jump", state.testSpaceJump, "TESTED", "TEST SPACE KEY");
    drawTestItem("Mouse Look", state.testMouseLook, "TESTED", "TEST MOUSE MOVEMENT");

    yOffset += 10;

    // **COMBAT SYSTEM SECTION**
    Vector2 combatTitlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)yOffset};
    UIDesign::drawStyledText("Combat System", combatTitlePos, UIDesign::getFontBody());

    // Section underline
    DrawRectangle((float)(panelX + UIDesign::getSpacingMedium()), (float)(yOffset + 18),
                  (float)(panelWidth - 2 * UIDesign::getSpacingMedium()), 2.0f,
                  UIDesign::fadeColor(UIDesign::getHealthColor(), 0.6f));
    yOffset += 30;

    drawTestItem("Melee Attack", state.testMeleeSwing, "TESTED", "TEST LEFT MOUSE CLICK");
    drawTestItem("Hit Detection", state.testMeleeHitDetection, "TESTED", "HIT TARGETS WITH ATTACKS");

    yOffset += 10;

    // **WORLD INTERACTION SECTION**
    Vector2 worldTitlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)yOffset};
    UIDesign::drawStyledText("World Interaction", worldTitlePos, UIDesign::getFontBody());

    // Section underline
    DrawRectangle((float)(panelX + UIDesign::getSpacingMedium()), (float)(yOffset + 18),
                  (float)(panelWidth - 2 * UIDesign::getSpacingMedium()), 2.0f,
                  UIDesign::fadeColor(UIDesign::getManaColor(), 0.6f));
    yOffset += 30;

    drawTestItem("Building Entry", state.testBuildingEntry, "TESTED", "OPEN A DOOR - WALK TO BUILDING");
    drawTestItem("NPC Dialog", state.testNPCInteraction, "TESTED", "TALK TO AN NPC - PRESS E");

    yOffset += 10;

    // **SYSTEM STATUS SECTION**
    Vector2 statusTitlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)yOffset};
    UIDesign::drawStyledText("System Status", statusTitlePos, UIDesign::getFontBody());

    // Section underline
    DrawRectangle((float)(panelX + UIDesign::getSpacingMedium()), (float)(yOffset + 18),
                  (float)(panelWidth - 2 * UIDesign::getSpacingMedium()), 2.0f,
                  UIDesign::fadeColor(UIDesign::getExperienceColor(), 0.6f));
    yOffset += 30;

    // Mouse state with dynamic color
    Color mouseStateColor = state.mouseReleased ? UIDesign::getTextWarning() : UIDesign::getTextHighlight();
    std::string mouseText = "Mouse State: " + std::string(state.mouseReleased ? "FREE" : "CAPTURED");
    Vector2 mousePos = {(float)(panelX + UIDesign::getSpacingLarge()), (float)yOffset};
    UIDesign::drawStyledText(mouseText, mousePos, {UIDesign::getFontSmall().size, mouseStateColor, false, 1.0f});
    yOffset += 18;

    // Location display
    std::string locationDisplayText = "Location: " + locationText;
    Vector2 locationPos = {(float)(panelX + UIDesign::getSpacingLarge()), (float)yOffset};
    UIDesign::drawStyledText(locationDisplayText, locationPos, {UIDesign::getFontSmall().size, locationColor, false, 1.0f});
    yOffset += 18;

    // Dialog status
    Color dialogColor = state.isInDialog ? UIDesign::getTextHighlight() : UIDesign::getTextSubtle();
    std::string dialogText = "Dialog: " + std::string(state.isInDialog ? "ACTIVE" : "INACTIVE");
    Vector2 dialogPos = {(float)(panelX + UIDesign::getSpacingLarge()), (float)yOffset};
    UIDesign::drawStyledText(dialogText, dialogPos, {UIDesign::getFontSmall().size, dialogColor, false, 1.0f});
    yOffset += 25;

    // **TESTING INSTRUCTIONS**
    Vector2 instructionsTitlePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)yOffset};
    UIDesign::drawStyledText("How to Test:", instructionsTitlePos, UIDesign::getFontBody());
    yOffset += 20;

    const char* instructions[] = {
        "• Move around with WASD keys",
        "• Look around with mouse",
        "• Jump with SPACE",
        "• Attack with LEFT MOUSE BUTTON",
        "• Walk to buildings and OPEN DOORS",
        "• Find NPCs inside and TALK TO THEM (press E)"
    };

    for (const char* instruction : instructions) {
        Vector2 instrPos = {(float)(panelX + UIDesign::getSpacingLarge()), (float)yOffset};
        UIDesign::drawStyledText(instruction, instrPos, UIDesign::getFontTiny());
        yOffset += 16;
    }

    // **CLOSE INSTRUCTIONS**
    Vector2 closePos = {(float)(panelX + UIDesign::getSpacingMedium()), (float)(panelY + panelHeight - 25)};
    UIDesign::drawStyledText("Press TAB again to close this testing panel", closePos, {12, UIDesign::getTextWarning(), false, 1.0f});

    // Add decorative elements
    UIDesign::drawRuneDecoration({(float)(panelX + 20), (float)(panelY + 20)}, 10.0f, UIDesign::getPrimaryLight());
    UIDesign::drawRuneDecoration({(float)(panelX + panelWidth - 30), (float)(panelY + 20)}, 10.0f, UIDesign::getPrimaryLight());
    UIDesign::drawScrollDecoration({(float)panelX, (float)(panelY + panelHeight - 15), (float)panelWidth, 15.0f}, UIDesign::getSecondaryLight());
}

// ===== COLLISION DETECTION & PREVENTION =====

void UISystemManager::clearOverlaps() {
    // Clear zone reservations to allow re-positioning
    zoneReservations_.clear();
}

Rectangle UISystemManager::calculateNonOverlappingPosition(UIZone preferredZone, int width, int height) const {
    Rectangle zoneBounds = getZoneBounds(preferredZone);

    // Check if the zone is already occupied
    if (isZoneOccupied(preferredZone)) {
        // Try alternative zones in order of preference
        std::vector<UIZone> alternatives;

        if (preferredZone == UIZone::TOP_LEFT) {
            alternatives = {UIZone::TOP_RIGHT, UIZone::BOTTOM_LEFT};
        } else if (preferredZone == UIZone::TOP_RIGHT) {
            alternatives = {UIZone::TOP_LEFT, UIZone::BOTTOM_RIGHT};
        } else if (preferredZone == UIZone::BOTTOM_LEFT) {
            alternatives = {UIZone::BOTTOM_RIGHT, UIZone::TOP_LEFT};
        } else if (preferredZone == UIZone::BOTTOM_RIGHT) {
            alternatives = {UIZone::BOTTOM_LEFT, UIZone::TOP_RIGHT};
        } else {
            alternatives = {UIZone::TOP_LEFT, UIZone::TOP_RIGHT, UIZone::BOTTOM_LEFT, UIZone::BOTTOM_RIGHT};
        }

        for (UIZone altZone : alternatives) {
            if (!isZoneOccupied(altZone)) {
                Rectangle altBounds = getZoneBounds(altZone);
                if (width <= altBounds.width && height <= altBounds.height) {
                    return altBounds;
                }
            }
        }

        // If all zones are occupied, return the original zone (will overlap)
        return zoneBounds;
    }

    return zoneBounds;
}
