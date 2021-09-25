#include "window.h"

#include <stdexcept>

namespace cgull {
    window::window() {
        if (!glfwInit()) {
            throw std::runtime_error("failed to initialize GLFW\n");
        }

        #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #endif
    
        auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfw_window = glfwCreateWindow(mode->width, mode->height, "CGull", nullptr, nullptr);
        if (!glfw_window) {
            glfwTerminate();
            throw std::runtime_error("failed to create window\n");
        }
        glfwMakeContextCurrent(glfw_window);

        glfwSetWindowUserPointer(glfw_window, this);

        glfwSetCharCallback(glfw_window, [](GLFWwindow* window, unsigned int codepoint) {
            static_cast<struct window*>(glfwGetWindowUserPointer(window))->pending_action = char_action{codepoint};
        });

        glfwSetKeyCallback(glfw_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (action != GLFW_RELEASE) {
                unsigned int km = static_cast<unsigned int>(key_mods::none);
                if (mods & GLFW_MOD_SHIFT) km |= static_cast<unsigned int>(key_mods::shift);
                if (mods & GLFW_MOD_CONTROL) km |= static_cast<unsigned int>(key_mods::ctrl);
                if (mods & GLFW_MOD_ALT) km |= static_cast<unsigned int>(key_mods::alt);
                if (mods & GLFW_MOD_SUPER) km |= static_cast<unsigned int>(key_mods::super);
                if (mods & GLFW_MOD_CAPS_LOCK) km |= static_cast<unsigned int>(key_mods::caps_lock);
                if (mods & GLFW_MOD_NUM_LOCK) km |= static_cast<unsigned int>(key_mods::num_lock);

                static_cast<struct window*>(glfwGetWindowUserPointer(window))->pending_action = special_key_action{static_cast<key_code>(key), km};
            }
        });

        glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* window, int button, int action, int mods) {
            if (action != GLFW_RELEASE && (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                coord c = {
                    static_cast<index>(xpos),
                    static_cast<index>(ypos)
                };
                mouse_button mb = button == GLFW_MOUSE_BUTTON_LEFT ? mouse_button::left : mouse_button::right;
                static_cast<struct window*>(glfwGetWindowUserPointer(window))->pending_action = click_action{c, mb};
            }
        });
    }

    std::optional<action> window::update() {
        if (pending_action.has_value()) {
            pending_action = std::nullopt; // reset the action so we don't execute it twice
        }

        glfwSwapBuffers(glfw_window);

        glfwWaitEvents();

        return pending_action;
    }

    bool window::should_close() const {
        return glfwWindowShouldClose(glfw_window);
    }
}