#pragma once

#include "application.h"

#define GLFW_INCLUDE_NONE
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#include <optional>

namespace cgull {

    struct window {
        window();
        std::optional<action> run();

    private:
        GLFWwindow* glfw_window;
        std::optional<action> pending_action;
    };
}