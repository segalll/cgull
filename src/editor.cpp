#include "editor.h"

#include <GLFW/glfw3.h>
#include <iostream>

void Editor::handleTextInput(unsigned int codepoint) {
    std::cout << (char)codepoint << "\n"; // TODO: make this enterCharacter
}

void Editor::handleSpecialKeyInput(int key, int mods) { // TODO: actually handle special inputs
    const bool ctrl = mods & GLFW_MOD_CONTROL;
    if (key == GLFW_KEY_V && ctrl) {
        std::cout << "paste\n";
    }
}

void Editor::handleMouseButton(int button, int mods, unsigned int xpos, unsigned int ypos) { // TODO: selections and stuff
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mState.mCursorPosition = screenPosToCoordinates(xpos, ypos);
    }
}

void Editor::enterCharacter(Char character) {} // TODO: implement this

Coordinates Editor::screenPosToCoordinates(unsigned int xpos, unsigned int ypos) const {  // TODO: implement this with character height?
    return Coordinates();
}