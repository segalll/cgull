#include "editor.h"

#include <QMatrix4x4>

#include <iostream>

Editor::Editor() {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    setFormat(format);
}

void Editor::initializeGL() {
    initializeOpenGLFunctions();

    m_vao.create();
    m_vao.bind();

    m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vbo.create();

    const std::vector<float> vertices = {
        100.0f, 50.0f,
        50.0f, 50.0f,
        50.0f, 100.0f,
        50.0f, 100.0f,
        100.0f, 100.0f,
        100.0f, 50.0f
    };

    m_vbo.bind();
    m_vbo.allocate(vertices.data(), vertices.size() * sizeof(float));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    m_text_shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                          "#version 330 core\n"
                                          "layout(location = 0) in vec2 a_vertex;\n"
                                          "layout (std140) uniform matrices {\n"
                                          "    mat4 proj;\n"
                                          "};\n"
                                          "void main() {\n"
                                          "    gl_Position = proj * vec4(a_vertex, 0.0, 1.0);\n"
                                          "}");
    m_text_shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                          "#version 330 core\n"
                                          "out vec4 f_color;\n"
                                          "void main() {\n"
                                          "    f_color = vec4(1.0, 1.0, 1.0, 1.0);\n"
                                          "}");
    m_text_shader.link();
    m_text_shader.bind();

    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glUniformBlockBinding(m_text_shader.programId(), m_text_shader.uniformLocation("matrices"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void Editor::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    proj.setToIdentity();
    proj.ortho(0, w, h, 0, -1, 1);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(proj), proj.constData(), GL_STATIC_DRAW);
}

void Editor::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
