#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "editor.h"
#include "renderer.h"
#include "window.h"

#include <iostream>
#include <thread>

int main() {
    cgull::window w;

    namespace key = cgull::key;
    using key_mods = cgull::key_mods;
    cgull::editor e(w.get_size(), {{key::seq(key::enter), "new-line"},
                                   {key::seq({key_mods::shift}, key::tab), "unindent"},
                                   {key::seq(key::tab), "indent"},
                                   {key::seq(key::backspace), "backspace"},
                                   {key::seq(key::left), "cursor-left"},
                                   {key::seq(key::right), "cursor-right"},
                                   {key::seq(key::up), "cursor-up"},
                                   {key::seq(key::down), "cursor-down"},
                                   {key::seq({key_mods::platform}, 's'), "save"},
                                   {key::seq({key_mods::platform}, 'r'), "compile"},
                                   {key::seq({key_mods::platform}, key::equal), "zoom-in"},
                                   {key::seq({key_mods::platform}, key::minus), "zoom-out"},
                                   {key::seq({key_mods::platform}, 'o'), "open"}});

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
            r.loop_cv.notify_one();
        }
    }
    r.loop_cv.notify_one();

    if (render_thread.joinable()) {
        render_thread.join();
    }

    return 0;
}
