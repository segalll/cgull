#include "editor.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <locale>

std::string toUtf8(char32_t codepoint) {
    auto& cvt = std::use_facet<std::codecvt<char32_t, char, std::mbstate_t>>(std::locale());

    std::mbstate_t state{};

    std::string out(cvt.max_length(), '\0');

    const char32_t* lastIn;
    char* lastOut;
    cvt.out(state, &codepoint, &codepoint + 1, lastIn, &out[0], &out[out.size()], lastOut);

    return out;
}

void Editor::handleTextInput(unsigned int codepoint) {
    std::cout << toUtf8(codepoint) << "\n";
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

void Editor::enterCharacter(Char character) { // TODO: implement this
    std::cout << character << std::endl;
}

Coordinates Editor::screenPosToCoordinates(unsigned int xpos, unsigned int ypos) const {  // TODO: implement this with character height?
    return Coordinates();
}