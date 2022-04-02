#ifndef EDITOR_H
#define EDITOR_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWindow>

class Editor : public QOpenGLWindow, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    Editor();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLShaderProgram m_text_shader;
    GLuint m_ubo;
    QMatrix4x4 proj;
};

#endif // EDITOR_H
