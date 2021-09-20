#pragma once

#include "application.h"

namespace cgull {
    void init_gl(auto glad_load_func);
    void draw(const application& app);
    void draw_text(const buffer& buf, coord size);
}