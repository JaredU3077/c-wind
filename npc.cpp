// npc.cpp
#include "npc.h"
#include <cmath>

// MAX_NPCS is now defined in npc.h
NPC npcs[MAX_NPCS];

// Note: NPCs are now initialized in Browserwind.cpp to avoid conflicts
// This function is kept for compatibility but should not be called
void initNPCs() {
    // NPCs are initialized in Browserwind.cpp with proper building positioning
    // This prevents conflicts between different initialization locations
}

bool checkNPCCollision(Vector3 capsulePos, float capsuleRadius, float capsuleHeight, int excludeNPC) {
    for (int n = 0; n < MAX_NPCS; n++) {
        if (n == excludeNPC) continue;

        const float npcRadius = 0.6f;
        const float npcHeight = 2.0f;

        float capsuleBottom = capsulePos.y;
        float capsuleTop = capsulePos.y + capsuleHeight;
        float npcBottom = npcs[n].position.y;
        float npcTop = npcs[n].position.y + npcHeight;

        if (capsuleTop < npcBottom || capsuleBottom > npcTop) continue;

        float dx = capsulePos.x - npcs[n].position.x;
        float dz = capsulePos.z - npcs[n].position.z;
        float distance2D = sqrtf(dx * dx + dz * dz);

        if (distance2D <= npcRadius + capsuleRadius) return true;
    }
    return false;
}