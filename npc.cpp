// npc.cpp
#include "npc.h"
#include <cmath>

// MAX_NPCS is now defined in npc.h
NPC npcs[MAX_NPCS];

// Initialize NPCs with proper building positioning
void initNPCs() {
    // Mayor White in Mayor's Building
    npcs[0] = {
        .position = {0.0f, 0.0f, -15.5f},
        .name = "Mayor White",
        .dialog = "Greetings, citizen! Welcome to my office. What brings you here today?",
        .color = WHITE,
        .canInteract = true,
        .interactionRadius = 2.5f
    };

    // Buster Shoppin in Shop Building (positioned inside the building)
    npcs[1] = {
        .position = {12.0f, 0.0f, -2.0f},  // Inside shop building, facing the door
        .name = "Buster Shoppin",
        .dialog = "Welcome to my shop! I've got all sorts of goods and supplies for sale.",
        .color = GREEN,
        .canInteract = true,
        .interactionRadius = 2.5f
    };
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