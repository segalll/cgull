#define GLFW_INCLUDE_NONE
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "window.h"
#include "editor.h"
#include "renderer.h"

#include <iostream>
#include <thread>

int main() {
    cgull::window w;
    cgull::editor e(w.get_size(), {
        {{'p'}, "move-up"}
    });
    cgull::renderer r(w.get_size(), &e.buf);

    w.renderer_ptr = &r;
    e.renderer_ptr = &r;

    r.render();

    w.swap_buffer(); // update the window after we drew the initial state

    glfwMakeContextCurrent(nullptr);

    auto render_thread = std::thread(cgull::render_loop, &r, w.glfw_window);
    render_thread.detach();

    while (!w.should_close()) {
        const auto actions = w.update();

        if (!actions.empty()) {
            e.update(actions);
            r.should_redraw = true;
        }
    }

    if (render_thread.joinable()) {
        render_thread.join();
    }

    return 0;
}
