#include "window.h"
#include "editor.h"
#include "renderer.h"

#include <iostream>
#include <thread>
#include <memory>

void render_loop(cgull::renderer r, GLFWwindow* glfw_window) {
    glfwMakeContextCurrent(glfw_window);

    while (true) {
        r.render();
    }
}

int main() {
    cgull::window w;
    cgull::editor e(w.get_size(), {
        {{'p'}, "move-up"}
    });
    cgull::renderer r(w.get_size(), std::move(std::unique_ptr<cgull::editor>(&e)));

    r.render();
    w.swap_buffer(); // update the window after we drew the initial state

    glfwMakeContextCurrent(nullptr);

    const auto render_thread = std::thread(render_loop, std::move(r), w.glfw_window);

    while (!w.should_close()) {
        const auto actions = w.update();

        if (!actions.empty()) {
            e.update(actions);
            //r.render();
        }
    }

    return 0;
}
