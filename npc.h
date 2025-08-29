// npc.h
#ifndef NPC_H
#define NPC_H

#include "raylib.h"
#include <string>

struct NPC {
    Vector3 position;
    std::string name;
    std::string dialog;
    Color color;
    bool canInteract;
    float interactionRadius;
};

extern const int MAX_NPCS;
extern NPC npcs[MAX_NPCS];

void initNPCs();
bool checkNPCCollision(Vector3 capsulePos, float capsuleRadius, float capsuleHeight, int excludeNPC = -1);

#endif