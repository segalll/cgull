#include "window.h"
#include "editor.h"
#include "renderer.h"

int main() {
    cgull::window w;
    cgull::renderer r;
    cgull::editor e({
        {{'p'}, "move-up"}
    });

    r.render(e);
    w.swap_buffer(); // update the window after we drew the initial state

    while (!w.should_close()) {
        const auto actions = w.update();

        if (!actions.empty()) {
            e.update(actions);
            r.render(e);
        }
    }

    return 0;
}
