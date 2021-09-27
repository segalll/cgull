#pragma once

#include "editor.h"

#define GLFW_INCLUDE_NONE
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#include <vector>

namespace cgull {

    struct window {
        window();
        std::vector<action> update();
        bool should_close() const;
        void swap_buffer(); // only for the initial buffer swap

    private:
        GLFWwindow* glfw_window;
        std::vector<action> pending_actions;
    };
}