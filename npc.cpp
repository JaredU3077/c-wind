// npc.cpp
#include "npc.h"
#include "math_utils.h"
#include "raylib.h"
#include <cmath>

// MAX_NPCS is now defined in npc.h
NPC npcs[MAX_NPCS];

// Initialize NPCs with proper building positioning
void initNPCs() {
    // Mayor White in Mayor's Building - positioned relative to building interior
    npcs[0] = {
        .position = {-12.0f, 0.0f, 0.0f},  // Inside Mayor's Office at building center
        .name = "Mayor White",
        .dialog = "Greetings, citizen! Welcome to my office. What brings you here today?",
        .color = WHITE,
        .canInteract = true,
        .interactionRadius = 2.5f
    };

    // Buster Shoppin in Shop Building - positioned relative to building interior
    npcs[1] = {
        .position = {0.0f, 0.0f, 12.0f},  // Inside General Store at building center
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

// Render NPC function
void renderNPC(NPC npc, Camera3D camera, float currentTime) {
    // Draw NPC body (cylinder)
    Vector3 npcPos = npc.position;
    DrawCylinder(npcPos, 0.6f, 0.6f, 2.0f, 16, npc.color);

    // Draw NPC head (sphere)
    Vector3 headPos = {npcPos.x, npcPos.y + 2.2f, npcPos.z};
    DrawSphere(headPos, 0.4f, npc.color);

    // Draw NPC name above head (convert 3D to 2D screen position)
    Vector3 namePos3D = {npcPos.x, npcPos.y + 3.0f, npcPos.z};
    Vector2 namePos2D = GetWorldToScreen(namePos3D, camera);
    DrawText(npc.name.c_str(), (int)namePos2D.x - 50, (int)namePos2D.y - 20, 16, npc.color);

    // Draw interaction indicator if player is close
    float distance = MathUtils::distance3D(npc.position, camera.position);

    if (distance <= npc.interactionRadius) {
        // Pulsing interaction indicator
        float pulse = 0.8f + sinf(currentTime * 3.0f) * 0.2f;
        Vector3 indicatorPos = {npcPos.x, npcPos.y + 3.5f, npcPos.z};
        DrawSphere(indicatorPos, 0.2f * pulse, YELLOW);
    }
}