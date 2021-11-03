#pragma once

#include "editor.h"
#include "renderer.h"

#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace cgull {
    struct window {
        window();
        std::vector<action> update();
        bool should_close() const;
        void swap_buffer(); // only for the initial buffer swap
        coord get_size();
        GLFWwindow* glfw_window;
        renderer* renderer_ptr;
        std::vector<action> pending_actions;
    };
}