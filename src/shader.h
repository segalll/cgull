#pragma once

#include <string>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    void use();

    void setUint(const std::string& name, unsigned int value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
private:
    unsigned int mProgram;

    void checkCompileErrors(unsigned int shader, const std::string& type);
};