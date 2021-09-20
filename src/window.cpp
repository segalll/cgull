#include "window.h"

#include "draw.h"

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
                key_mods km;
                switch (mods) {
                    case GLFW_MOD_SHIFT:
                        km = key_mods::shift;
                        break;
                    case GLFW_MOD_CONTROL:
                        km = key_mods::ctrl;
                        break;
                    case GLFW_MOD_ALT:
                        km = key_mods::alt;
                        break;
                    case GLFW_MOD_SUPER:
                        km = key_mods::super;
                        break;
                    case GLFW_MOD_CAPS_LOCK:
                        km = key_mods::caps_lock;
                        break;
                    case GLFW_MOD_NUM_LOCK:
                        km = key_mods::num_lock;
                        break;
                    default:
                        km = key_mods::none;
                        break;
                }
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

        init_gl(glfwGetProcAddress); // I don't want to include glfw in draw.h
    }

    std::optional<action> window::run() {
        if (pending_action.has_value()) {
            pending_action = std::nullopt; // reset the action so we don't execute it twice
        }

        glfwSwapBuffers(glfw_window);

        glfwWaitEvents();

        return pending_action;
    }
}