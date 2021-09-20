#include "application.h"

#include <glad/gl.h>

#include <stdexcept>

namespace cgull {
    void init_gl(auto glad_load_func) {
        if (gladLoadGL(glad_load_func) == 0) {
            throw std::runtime_error("failed to initialize OpenGL context\n");
        }
    }
}