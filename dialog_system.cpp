// dialog_system.cpp
#include "dialog_system.h"

void startDialog(int npcIndex, GameState& state) {
    state.isInDialog = true;
    state.currentNPC = npcIndex;
    state.showDialogWindow = true;

    if (!state.mouseReleased) {
        EnableCursor();
    }

    switch (npcIndex) {
        case 0:
            state.dialogText = "Mayor White: \"Greetings, citizen! Welcome to my office. What brings you here today?\"";
            state.dialogOptions[0] = "Ask about town affairs";
            state.dialogOptions[1] = "Request town assistance";
            state.dialogOptions[2] = "Goodbye";
            state.numDialogOptions = 3;
            break;
        case 1:
            state.dialogText = "Buster Shoppin: \"Welcome to my shop! I've got all sorts of goods and supplies for sale.\"";
            state.dialogOptions[0] = "Browse general supplies";
            state.dialogOptions[1] = "Ask about special items";
            state.dialogOptions[2] = "Goodbye";
            state.numDialogOptions = 3;
            break;
    }
}

void handleDialogOption(int optionIndex, GameState& state) {
    if (state.currentNPC == -1) return;

    switch (state.currentNPC) {
        case 0:
            if (optionIndex == 0) {
                state.dialogText = "Mayor White: \"Town affairs are going well! We're always looking for capable adventurers to help with local issues.\"";
            } else if (optionIndex == 1) {
                state.dialogText = "Mayor White: \"What kind of assistance do you need? I can provide information or direct you to the right people.\"";
            } else {
                state.dialogText = "Mayor White: \"Thank you for visiting. Please come back if you need anything else.\"";
                state.isInDialog = false;
                state.showDialogWindow = false;
                if (!state.mouseReleased) {
                    DisableCursor();
                }
            }
            break;
        case 1:
            if (optionIndex == 0) {
                state.dialogText = "Buster Shoppin: \"I have food, water, rope, torches, and basic adventuring supplies. What catches your eye?\"";
            } else if (optionIndex == 1) {
                state.dialogText = "Buster Shoppin: \"I sometimes get rare items from traveling merchants. Check back later for special deals!\"";
            } else {
                state.dialogText = "Buster Shoppin: \"Thanks for shopping! Come back anytime you need supplies.\"";
                state.isInDialog = false;
                state.showDialogWindow = false;
                if (!state.mouseReleased) {
                    DisableCursor();
                }
            }
            break;
    }

    if (state.isInDialog) {
        state.dialogOptions[0] = "Continue talking";
        state.dialogOptions[1] = "Ask another question";
        state.dialogOptions[2] = "Goodbye";
        state.numDialogOptions = 3;
    }
}