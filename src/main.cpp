#define GLFW_INCLUDE_NONE
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include "editor.h"
#include "renderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    auto videoMode = glfwGetVideoMode(primaryMonitor);

    GLFWwindow* window = glfwCreateWindow(videoMode->width, videoMode->height, "CGull", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cout << "Failed to initialize OpenGL context\n";
        return -1;
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // TODO: abstract this into editor background color

    Editor editor;
    Renderer renderer;

    glfwSetWindowUserPointer(window, &editor);

    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint) {
        static_cast<Editor*>(glfwGetWindowUserPointer(window))->handleTextInput(codepoint);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action != GLFW_RELEASE) {
            static_cast<Editor*>(glfwGetWindowUserPointer(window))->handleSpecialKeyInput(key, mods);
        }
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        if (action != GLFW_RELEASE) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            static_cast<Editor*>(glfwGetWindowUserPointer(window))->handleMouseButton(
                button,
                mods,
                static_cast<unsigned int>(xpos),
                static_cast<unsigned int>(ypos)
            );
        }
    });

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwWaitEvents();
    }

    glfwTerminate();

    return 0;
}
