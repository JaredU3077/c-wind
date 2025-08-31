// ui_system.h
#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include "raylib.h"
#include "game_state.h"
#include <string>
#include <map>

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

// Initialization function for main.cpp
void initializeUISystem();
void shutdownUISystem();

#endif // UI_SYSTEM_H
