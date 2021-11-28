#include "shader.h"

#include <glad/gl.h>

#include <fstream>
#include <iostream>
#include <sstream>

void check_compile_errors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "Shader failed to compile (type: " << type << ")\n"
                      << infoLog << "\n\n";
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "Program failed to link (type: " << type << ")\n"
                      << infoLog << "\n\n";
        }
    }
}

namespace cgull {
unsigned int create_shader(const std::string& vertex_path,
                           const std::string& fragment_path) {
    std::string vertex_code, fragment_code;
    std::ifstream v_shader_file, f_shader_file;

    v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        v_shader_file.open(vertex_path);
        f_shader_file.open(fragment_path);
        std::stringstream v_shader_stream, f_shader_stream;

        v_shader_stream << v_shader_file.rdbuf();
        f_shader_stream << f_shader_file.rdbuf();

        v_shader_file.close();
        f_shader_file.close();

        vertex_code = v_shader_stream.str();
        fragment_code = f_shader_stream.str();
    } catch (std::ifstream::failure e) {
        throw std::runtime_error("failed to read shader file");
    }

    const char* v_shader_code = vertex_code.c_str();
    const char* f_shader_code = fragment_code.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, nullptr);
    glCompileShader(vertex);
    check_compile_errors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, nullptr);
    glCompileShader(fragment);
    check_compile_errors(fragment, "FRAGMENT");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    check_compile_errors(program, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}
} // namespace cgull