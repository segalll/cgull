#include "editor.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit())
        return -1;

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3)
    glfwWindowHint(GLFW_CONTENT_VERSION_MINOR, 2)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)
#endif

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    int width, height;
    glfwGetMonitorPhysicalSize(primaryMonitor, &width, &height);

    GLFWwindow* window = glfwCreateWindow(width, height, "CGull", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
