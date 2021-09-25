#include "window.h"
#include "editor.h"
#include "renderer.h"

int main() {
    cgull::window w;
    cgull::editor a({
        {{'p'}, "move-up"}
    });
    cgull::renderer r;

    while (!w.should_close()) {
        const auto action = w.update();

        if (action.has_value()) {
            a.update(action.value());
            r.render(a);
        }
    }

    return 0;
}
