#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "raylib.h"
#include "game_state.h"
#include "environment_manager.h"
#include "performance.h"
#include "constants.h"
#include "render_utils.h"  // For renderBuildingInterior, renderNPC, etc.
#include "ui_system.h"     // For g_uiSystem
#include "npc.h"           // For MAX_NPCS, npcs
#include "combat.h"        // For renderCombat

class RenderSystem {
public:
    RenderSystem(GameState& state, EnvironmentManager& environment, AdvancedFrameStats& performanceStats);
    void renderAll(const Camera3D& camera, float time);

private:
    GameState& state_;
    EnvironmentManager& environment_;
    AdvancedFrameStats& performanceStats_;

    void render3DWorld(const Camera3D& camera, float time);
    void render3DInteractions(const Camera3D& camera);
    void render2DOverlays(const Camera3D& camera, float time);
    std::string getLocationText() const;
    std::string getJumpText() const;
};

#endif // RENDER_SYSTEM_H