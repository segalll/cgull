#include "window.h"

#include <stdexcept>
#include <iostream>

namespace cgull {
    window::window() {
        if (!glfwInit()) {
            throw std::runtime_error("failed to initialize GLFW\n");
        }

        #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
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
            static_cast<struct window*>(
                glfwGetWindowUserPointer(window)
            )->pending_actions.push_back(char_action{codepoint});
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

                static_cast<struct window*>(
                    glfwGetWindowUserPointer(window)
                )->pending_actions.push_back(special_key_action{static_cast<key_code>(key), km});
            }
        });

        glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* window, int button, int action, int mods) {
            if (action != GLFW_RELEASE && (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                coord c = {
                    static_cast<index>(ypos),
                    static_cast<index>(xpos)
                };
                mouse_button mb = button == GLFW_MOUSE_BUTTON_LEFT ? mouse_button::left : mouse_button::right;
                static_cast<struct window*>(
                    glfwGetWindowUserPointer(window)
                )->pending_actions.push_back(click_action{c, mb});
            }
        });

        glfwSetFramebufferSizeCallback(glfw_window, [](GLFWwindow* window, int width, int height) {
            coord c = {
                static_cast<index>(height),
                static_cast<index>(width)
            };

            struct window* window_ptr = static_cast<struct window*>(glfwGetWindowUserPointer(window));

            window_ptr->renderer_ptr->window_size = c;
            window_ptr->renderer_ptr->should_redraw = true;
            window_ptr->pending_actions.push_back(resize_action{c});
        });
    }

    std::vector<action> window::update() {
        if (!pending_actions.empty()) {
            pending_actions.clear();
        }

        glfwWaitEvents();

        return pending_actions;
    }

    bool window::should_close() const {
        return glfwWindowShouldClose(glfw_window);
    }

    void window::swap_buffer() {
        glfwSwapBuffers(glfw_window);
    }

    coord window::get_size() {
        int width, height;
        glfwGetWindowSize(glfw_window, &width, &height);
        return coord{
            static_cast<index>(height),
            static_cast<index>(width)
        };
    }
}