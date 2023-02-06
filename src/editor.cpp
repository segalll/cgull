#include "editor.h"

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>

#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QPushButton>
#include <QStandardPaths>
#include <QToolTip>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace {
constexpr bool isQuote(char character) { return character == '"' || character == '\''; }

constexpr bool isBracket(char character) { return character == '(' || character == '{' || character == '['; }

constexpr bool isClosingBracket(char character) { return character == ')' || character == '}' || character == ']'; }

constexpr char correspondingBracket(char bracket) { return bracket == '(' ? ')' : bracket + 2; }

constexpr bool isContainer(char character) {
    return isBracket(character) || character == '"' || character == '\'';
}

constexpr bool isClosingContainer(char character) { return isQuote(character) || isClosingBracket(character); }

constexpr char correspondingContainer(char character) {
    return isQuote(character) ? character : correspondingBracket(character);
}
} // namespace

Editor::Editor() {
    // necessary for keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // enables compressed keyboard input
    setAttribute(Qt::WA_KeyCompression, true);

    QRect r(0, 0, 800, 600);
    r.moveCenter(screen()->geometry().center());
    setGeometry(r);

    m_renderData.width = 800;
    m_renderData.height = 600;

    m_renderData.xOffset = 60.0f;
    m_renderData.yOffset = 70.0f;

    setCursor(Qt::IBeamCursor);

    QPushButton* p = new QPushButton("Compile", this);
    p->setGeometry(20, 35, 75, 35);
    p->setCursor(Qt::ArrowCursor);
    p->show();

    connect(p, &QPushButton::clicked, this, [this]() {
        compile(m_currentFileName);
    });

    m_fileTabBar = new QTabBar(this);
    m_fileTabBar->setGeometry(5, 5, 790, 10);
    m_fileTabBar->setCursor(Qt::ArrowCursor);
    m_fileTabBar->setExpanding(true);
    m_fileTabBar->setTabsClosable(true);
    m_fileTabBar->addTab("untitled");

    connect(m_fileTabBar, &QTabBar::tabBarClicked, this, &Editor::tabChanged);
    connect(m_fileTabBar, &QTabBar::tabCloseRequested, this, &Editor::tabClosed);

    m_compileProcess = new QProcess(this);
    m_compileProcess->setEnvironment(QProcess::systemEnvironment());

    connect(m_compileProcess, &QProcess::finished, this, &Editor::compileFinished);

    m_lintProcess = new QProcess(this);
    m_lintProcess->setEnvironment(QProcess::systemEnvironment());

    connect(m_lintProcess, &QProcess::finished, this, &Editor::lintFinished);

    m_textBuffer = {""};

    m_tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/";
    QDir dir;

    if (!dir.exists(m_tempPath)) {
        dir.mkpath(m_tempPath);
    }
}

void Editor::initializeGL() {
    initializeOpenGLFunctions();

    m_renderData.text.vao.create();
    m_renderData.text.vao.bind();

    m_renderData.text.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_renderData.text.vbo.create();
    m_renderData.text.vbo.bind();
    m_renderData.text.vbo.allocate(nullptr, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)(2 * sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)(4 * sizeof(float)));

    m_renderData.text.shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                          "#version 330 core\n"
                                          "layout (location = 0) in vec2 aVertex;\n"
                                          "layout (location = 1) in vec2 aTexcoord;\n"
                                          "layout (location = 2) in vec3 aColor;\n"
                                          "layout (std140) uniform matrices {\n"
                                          "    mat4 proj;\n"
                                          "};\n"
                                          "out vec2 vTexcoord;\n"
                                          "out vec3 vColor;\n"
                                          "uniform vec2 scroll;\n"
                                          "void main() {\n"
                                          "    vTexcoord = aTexcoord;\n"
                                          "    vColor = aColor;\n"
                                          "    gl_Position = proj * vec4(aVertex.x + scroll.x, aVertex.y - scroll.y, 0.0, 1.0);\n"
                                          "}");
    m_renderData.text.shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                          "#version 330 core\n"
                                          "in vec2 vTexcoord;\n"
                                          "in vec3 vColor;\n"
                                          "out vec4 fColor;\n"
                                          "uniform sampler2D atlas;\n"
                                          "void main() {\n"
                                          "    fColor = vec4(vColor, texture(atlas, vTexcoord).r);\n"
                                          "}");
    m_renderData.text.shader.link();
    m_renderData.text.shader.bind();

    glGenTextures(1, &m_renderData.text.texture);

    m_renderData.cursor.vao.create();
    m_renderData.cursor.vao.bind();

    m_renderData.cursor.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_renderData.cursor.vbo.create();
    m_renderData.cursor.vbo.bind();
    m_renderData.cursor.vbo.allocate(nullptr, 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    m_renderData.cursor.shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                          "#version 330 core\n"
                                          "layout (location = 0) in vec2 aVertex;\n"
                                          "layout (std140) uniform matrices {\n"
                                          "    mat4 proj;\n"
                                          "};\n"
                                          "uniform vec2 scroll;\n"
                                          "void main() {\n"
                                          "    gl_Position = proj * vec4(aVertex.x + scroll.x, aVertex.y - scroll.y, 0.0, 1.0);\n"
                                          "}");
    m_renderData.cursor.shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                          "#version 330 core\n"
                                          "out vec4 fColor;\n"
                                          "uniform vec4 color;\n"
                                          "void main() {\n"
                                          "    fColor = color;\n"
                                          "}");
    m_renderData.cursor.shader.link();
    m_renderData.cursor.shader.bind();

    glGenBuffers(1, &m_renderData.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);
    glUniformBlockBinding(m_renderData.text.shader.programId(), m_renderData.text.shader.uniformLocation("matrices"), 0);
    glUniformBlockBinding(m_renderData.cursor.shader.programId(), m_renderData.cursor.shader.uniformLocation("matrices"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_renderData.ubo);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    loadFontGlyphs();
}

void Editor::resizeGL(int w, int h) {
    qDebug() << w << " " << h;
    glViewport(0, 0, w, h);
    glScissor(0, 0, w, h - (int)m_renderData.yOffset + 2);
    glDisable(GL_SCISSOR_TEST); // must be able to clear new area

    m_renderData.proj.setToIdentity();
    m_renderData.proj.ortho(0, w, h, 0, -1, 1);
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(m_renderData.proj), m_renderData.proj.constData(), GL_STATIC_DRAW);

    m_renderData.width = w;
    m_renderData.height = h;

    m_fileTabBar->setGeometry(5, 5, w - 10, 20);
}

void Editor::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST); // must happen after clear

    m_renderData.scroll = qBound(0.0f, m_renderData.scroll, maxScroll());

    renderText();
    renderError();
    renderSelection();
    renderCursor();
    renderLineNumbers();
}

bool Editor::event(QEvent* ev) {
    if (ev->type() == QEvent::ToolTip && m_lints.size() > 0) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(ev);

        for (std::size_t i = 0; i < m_lints.size(); i++) {
            QRect lintHoverRegion = getLintHoverRegion(i);
            if (lintHoverRegion.contains(helpEvent->pos())) {
                QToolTip::showText(pos() + lintHoverRegion.bottomRight() - QPoint(10, 10), m_lints[i].error);
                return true;
            }
        }
        QToolTip::hideText();
        ev->ignore();

        return true;
    }
    return QWidget::event(ev);
}

void Editor::keyPressEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Left) {
        cursorLeft();
    } else if (ev->key() == Qt::Key_Right) {
        cursorRight();
    } else if (ev->key() == Qt::Key_Up) {
        cursorUp();
    } else if (ev->key() == Qt::Key_Down) {
        cursorDown();
    } else if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
        newLine();
        lint();
        m_currentFileChanged = true;
    } else if (ev->key() == Qt::Key_Tab) {
        indent();
        lint();
        m_currentFileChanged = true;
    } else if (ev->key() == Qt::Key_Backtab) {
        unindent();
        lint();
        m_currentFileChanged = true;
    } else if (ev->key() == Qt::Key_Backspace) {
        backspace();
        lint();
        m_currentFileChanged = true;
    } else if (ev->matches(QKeySequence::SelectAll)) {
        selectAll();
    } else if (ev->matches(QKeySequence::Copy)) {
        copy();
    } else if (ev->matches(QKeySequence::Paste)) {
        paste();
        lint();
        m_currentFileChanged = true;
    } else if (ev->matches(QKeySequence::Undo)) {
        undo();
        lint();
        m_currentFileChanged = true;
    } else if (ev->matches(QKeySequence::Redo)) {
        redo();
        lint();
        m_currentFileChanged = true;
    } else if (ev->text().length() > 0) {
        enterStr(ev->text().toStdString());
        lint();
        m_currentFileChanged = true;
    }
    update(); // schedule redraw hopefully
}

void Editor::mousePressEvent(QMouseEvent* ev) {
    m_cursorState.pos = mouseToCursorPos(ev->position().x(), ev->position().y());
    m_cursorState.selectionStart = std::nullopt;
    if (m_cursorState.pos.row > m_textBuffer.size()) {
        m_cursorState.pos.row = m_textBuffer.size() - 1;
        m_cursorState.pos.col = m_textBuffer.back().length();
    }
    m_cursorState.mouseDown = true;
    update(); // schedule redraw
}

void Editor::mouseReleaseEvent(QMouseEvent* ev) {
    Q_UNUSED(ev);
    m_cursorState.mouseDown = false;
}

// PLEASE REFACTOR THIS FUNCTION
void Editor::mouseDoubleClickEvent(QMouseEvent* ev) {
    Coord c = mouseToCursorPos(ev->position().x(), ev->position().y());
    if (m_textBuffer[c.row].size() == 0) return;
    if (c.col >= m_textBuffer[c.row].size()) { // catherine is so cool and awesome henry is kinda cool I guess...
        c.col--;
    }
    if (std::isalnum(m_textBuffer[c.row][c.col]) || m_textBuffer[c.row][c.col] == '_') {
        m_cursorState.selectionStart = Coord { c.row, 0 };
        for (int i = c.col - 1; i >= 0; i--) {
            if (!std::isalnum(m_textBuffer[c.row][i]) && m_textBuffer[c.row][i] != '_') {
                m_cursorState.selectionStart = Coord { c.row, static_cast<unsigned int>(i + 1) };
                break;
            }
        }
        m_cursorState.pos = Coord { c.row, static_cast<unsigned int>(m_textBuffer[c.row].length()) };
        for (std::size_t i = c.col + 1; i < m_textBuffer[c.row].length(); i++) {
            if (!std::isalnum(m_textBuffer[c.row][i]) && m_textBuffer[c.row][i] != '_') {
                m_cursorState.pos = Coord { c.row, static_cast<unsigned int>(i) };
                break;
            }
        }
    } else if (m_textBuffer[c.row][c.col] == ' ') {
        m_cursorState.selectionStart = Coord { c.row, 0 };
        for (int i = c.col - 1; i >= 0; i--) {
            if (m_textBuffer[c.row][i] != ' ') {
                m_cursorState.selectionStart = Coord { c.row, static_cast<unsigned int>(i + 1) };
                break;
            }
        }
        m_cursorState.pos = Coord { c.row, static_cast<unsigned int>(m_textBuffer[c.row].length()) };
        for (std::size_t i = c.col + 1; i < m_textBuffer[c.row].length(); i++) {
            if (m_textBuffer[c.row][i] != ' ') {
                m_cursorState.pos = Coord { c.row, static_cast<unsigned int>(i) };
                break;
            }
        }
    } else {
        m_cursorState.selectionStart = c;
        m_cursorState.pos = Coord { c.row, c.col + 1 };
    }
    update();
}

void Editor::mouseMoveEvent(QMouseEvent* ev) {
    if (!m_cursorState.mouseDown) return;

    Coord newCursor = mouseToCursorPos(ev->position().x(), ev->position().y());
    if (newCursor != m_cursorState.pos) {
        if (m_cursorState.selectionStart == std::nullopt) {
            m_cursorState.selectionStart = m_cursorState.pos;
            update();
        } else {
            update();
        }
        m_cursorState.pos = newCursor;
    }
}

void Editor::wheelEvent(QWheelEvent* ev) {
    m_renderData.scroll -= ev->pixelDelta().y();
    update(); // schedule redraw
}

void Editor::closeEvent(QCloseEvent* ev) {
    Q_UNUSED(ev);

    for (int i = 0; i <= m_fileTabBar->count(); i++) { // <= instead of < because untitled is created on tab close in tabClosed()
        m_fileTabBar->removeTab(0);
    }

    m_bufferCache.erase(m_currentFileName);

    m_cursorState = CursorState {
        { 0, 0 }, // cursor coord
        std::nullopt, // selection start
        false // mouse down
    };
    m_currentFileChanged = false;
    m_currentFileName = "untitled";
    m_fileTabBar->addTab("untitled");
    m_textBuffer = {""};
    m_lints.clear();

    update();
}

void Editor::renderText() {
    float scroll[] = { 0.0f, m_renderData.scroll };

    TextRenderData& t = m_renderData.text;

    t.vertices = generateBatchedVertices(m_textBuffer);
    t.vao.bind();
    glBindTexture(GL_TEXTURE_2D, t.texture);
    t.shader.bind();
    t.shader.setUniformValueArray(t.shader.uniformLocation("scroll"), scroll, 1, 2);
    t.vbo.bind();
    t.vbo.allocate(t.vertices.data(), t.vertices.size() * sizeof(TextVertex));
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);

    glDrawArrays(GL_TRIANGLES, 0, t.vertices.size());
}

void Editor::renderError() {
    if (m_lints.size() == 0) {
        return;
    }

    float scroll[] = { 0.0f, m_renderData.scroll };

    CursorRenderData& c = m_renderData.cursor; // reusing cursor render objects

    std::vector<float> errorVertices;

    for (const auto& l : m_lints) {
        float startX = properCursorPos(l.pos.col, l.pos.row);
        float endX = properCursorPos(l.pos.col + 1, l.pos.row);
        float y = m_fontData.height * (l.pos.row + 1) + m_renderData.yOffset + 2.0f;

        errorVertices.insert(errorVertices.end(), {
            endX, y,
            startX, y,
            startX, y + 2,
            startX, y + 2,
            endX, y + 2,
            endX, y
        });
    }

    c.vao.bind();
    c.shader.bind();
    float errorColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    c.shader.setUniformValueArray(c.shader.uniformLocation("color"), errorColor, 1, 4);
    c.shader.setUniformValueArray(c.shader.uniformLocation("scroll"), scroll, 1, 2);
    c.vbo.bind();
    c.vbo.allocate(errorVertices.data(), errorVertices.size() * sizeof(float));
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);

    glDrawArrays(GL_TRIANGLES, 0, errorVertices.size() / 2);
}

void Editor::renderSelection() {
    if (m_cursorState.selectionStart == std::nullopt || m_cursorState.selectionStart == m_cursorState.pos) {
        return;
    }

    float scroll[] = { 0.0f, m_renderData.scroll };

    CursorRenderData& c = m_renderData.cursor; // reusing cursor render objects

    std::vector<float> selectionVertices;

    auto [front, back] = getSelectionEnds();
    for (std::size_t r = front.row; r <= back.row; r++) {
        float startX, endX;
        if (r == front.row) {
            startX = properCursorPos(front.col, front.row);
        } else {
            startX = m_renderData.xOffset;
        }
        if (r == back.row) {
            endX = properCursorPos(back.col, back.row);
        } else {
            endX = properCursorPos(m_textBuffer[r].size(), r) + 12.0f;
        }

        float yPos = m_fontData.height * (r + 0.25) + m_renderData.yOffset;
        selectionVertices.insert(selectionVertices.end(), {
            endX, yPos,
            startX, yPos,
            startX, yPos + m_fontData.height,
            startX, yPos + m_fontData.height,
            endX, yPos + m_fontData.height,
            endX, yPos
        });
    }

    c.vao.bind();
    c.shader.bind();
    float selectionColor[] = { 1.0f, 1.0f, 1.0f, 0.3f };
    c.shader.setUniformValueArray(c.shader.uniformLocation("color"), selectionColor, 1, 4);
    c.shader.setUniformValueArray(c.shader.uniformLocation("scroll"), scroll, 1, 2);
    c.vbo.bind();
    c.vbo.allocate(selectionVertices.data(), selectionVertices.size() * sizeof(float));
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);

    glDrawArrays(GL_TRIANGLES, 0, selectionVertices.size() / 2);
}

void Editor::renderCursor() {
    float scroll[] = { 0.0f, m_renderData.scroll };

    CursorRenderData& c = m_renderData.cursor;

    float xPos = properCursorPos(m_cursorState.pos.col, m_cursorState.pos.row);
    float yPos = m_renderData.cursor.height + m_renderData.yOffset + (m_fontData.height * m_cursorState.pos.row) - m_fontData.height - 1.0f;

    const std::vector<float> cursorVertices = {
        xPos + 1.0f, yPos,
        xPos - 1.0f, yPos,
        xPos - 1.0f, yPos + m_renderData.cursor.height,
        xPos - 1.0f, yPos + m_renderData.cursor.height,
        xPos + 1.0f, yPos + m_renderData.cursor.height,
        xPos + 1.0f, yPos,
    };
    c.vao.bind();
    c.shader.bind();
    float cursorColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    c.shader.setUniformValueArray(c.shader.uniformLocation("color"), cursorColor, 1, 4);
    c.shader.setUniformValueArray(c.shader.uniformLocation("scroll"), scroll, 1, 2);
    c.vbo.bind();
    c.vbo.allocate(cursorVertices.data(), cursorVertices.size() * sizeof(float));
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);

    glDrawArrays(GL_TRIANGLES, 0, cursorVertices.size() / 2);
}

void Editor::renderLineNumbers() {
    // BACKGROUND RENDERING

    float scroll[] = { 0.0f, 0.0f };

    CursorRenderData& c = m_renderData.cursor; // reusing cursor render objects

    const std::vector<float> backgroundVertices = {
        50.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, (float)m_renderData.height,
        0.0f, (float)m_renderData.height,
        50.0f, (float)m_renderData.height,
        50.0f, 0.0f
    };
    c.vao.bind();
    c.shader.bind();
    float shade = m_currentFileChanged ? 0.3f : 0.15f;
    float backgroundColor[] = { shade, shade, shade, 1.0f };
    c.shader.setUniformValueArray(c.shader.uniformLocation("color"), backgroundColor, 1, 4);
    c.shader.setUniformValueArray(c.shader.uniformLocation("scroll"), scroll, 1, 2);
    c.vbo.bind();
    c.vbo.allocate(backgroundVertices.data(), backgroundVertices.size() * sizeof(float));
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);

    glDrawArrays(GL_TRIANGLES, 0, backgroundVertices.size() / 2);

    // LINE NUMBER RENDERING

    scroll[1] = m_renderData.scroll;

    TextRenderData& t = m_renderData.text;

    const std::vector<TextVertex> v = generateLineNumberVertices();
    t.vao.bind();
    glBindTexture(GL_TEXTURE_2D, t.texture);
    t.shader.bind();
    t.shader.setUniformValueArray(t.shader.uniformLocation("scroll"), scroll, 1, 2);
    t.vbo.bind();
    t.vbo.allocate(v.data(), v.size() * sizeof(TextVertex));
    glBindBuffer(GL_UNIFORM_BUFFER, m_renderData.ubo);

    glDrawArrays(GL_TRIANGLES, 0, v.size());
}

void Editor::enterStr(const std::string& str) {
    if (m_cursorState.selectionStart == std::nullopt) {
        std::string insertion = str;
        if (isClosingContainer(str[0]) && m_textBuffer[m_cursorState.pos.row].size() > m_cursorState.pos.col &&
            m_textBuffer[m_cursorState.pos.row][m_cursorState.pos.col] == str[0]) {
            insertion = {};
        } else if (isContainer(str[0])) {
            insertion += correspondingContainer(str[0]);
        }

        m_textBuffer[m_cursorState.pos.row].insert(m_cursorState.pos.col, insertion);
        m_cursorState.pos.col += str.length();
    } else {
        if (isContainer(str[0])) {
            auto [front, back] = getSelectionEnds();
            m_textBuffer[front.row].insert(front.col, { str[0] });
            m_cursorState.selectionStart->col += 1;
            m_textBuffer[back.row].insert(back.col + 1, { correspondingContainer(str[0]) });
            m_cursorState.pos.col += 1;
        } else {
            deleteSelection();
            m_textBuffer[m_cursorState.pos.row].insert(m_cursorState.pos.col, str);
            m_cursorState.pos.col += str.length();
        }
    }
}

void Editor::newLine() {
    int indentPos = m_textBuffer[m_cursorState.pos.row].find_first_not_of(' ');
    indentPos = indentPos == -1 ? m_textBuffer[m_cursorState.pos.row].length() : indentPos;

    std::string indent;
    indent.insert(0, indentPos, ' ');
    if (m_cursorState.selectionStart != std::nullopt) {
        deleteSelection();
    }
    m_textBuffer.insert(m_textBuffer.begin() + m_cursorState.pos.row + 1, indent + m_textBuffer[m_cursorState.pos.row].substr(m_cursorState.pos.col));
    m_textBuffer[m_cursorState.pos.row].erase(m_textBuffer[m_cursorState.pos.row].begin() + m_cursorState.pos.col, m_textBuffer[m_cursorState.pos.row].end());
    if (m_cursorState.pos.col > 0 && m_textBuffer[m_cursorState.pos.row][m_cursorState.pos.col - 1] == '{' &&  m_textBuffer[m_cursorState.pos.row + 1][indentPos] == '}') {
        m_textBuffer.insert(m_textBuffer.begin() + m_cursorState.pos.row + 1, indent + "    ");
        m_cursorState.pos.col = m_textBuffer[m_cursorState.pos.row + 1].length();
    } else {
        m_cursorState.pos.col = indentPos;
    }
    m_cursorState.pos.row += 1;
}

void Editor::indent() {
    auto [front, back] = getSelectionEnds(); // returns both cursor if no selection
    for (std::size_t r = front.row; r <= back.row; r++) {
        int indentPos = m_textBuffer[r].find_first_not_of(' ');
        indentPos = indentPos == -1 ? m_textBuffer[r].length() : indentPos;

        int toInsert = indentPos == 0 ? 4 : 4 - indentPos % 4;
        std::string indent;
        indent.insert(0, toInsert, ' ');
        m_textBuffer[r].insert(0, indent);

        if ((r == front.row && front == m_cursorState.pos) || (r == back.row && back == m_cursorState.pos)) {
            m_cursorState.pos.col += toInsert;
        } else if ((r == front.row && m_cursorState.selectionStart != std::nullopt && front == *m_cursorState.selectionStart) ||
                   (r == back.row && m_cursorState.selectionStart != std::nullopt && back == *m_cursorState.selectionStart)) {
            m_cursorState.selectionStart->col += toInsert;
        }
    }
}

bool Editor::unindent() {
    auto [front, back] = getSelectionEnds();
    for (std::size_t r = front.row; r <= back.row; r++) {
        int indentPos = m_textBuffer[r].find_first_not_of(' ', m_cursorState.pos.col);
        indentPos = indentPos == -1 ? m_textBuffer[r].length() : indentPos;
        if (indentPos > 0 && indentPos % 4 == 0 && m_textBuffer[m_cursorState.pos.row].substr(indentPos - 4, 4) == "    ") {
            m_textBuffer[m_cursorState.pos.row].erase(indentPos - 4, 4);
            m_cursorState.pos.col = m_cursorState.pos.col < 4 ? 0 : m_cursorState.pos.col - 4;
            if (front.row == back.row) return true;
        }
    }
    return false;
}

void Editor::deleteSelection() {
    auto [front, back] = getSelectionEnds();
    const int deleteCount = back.row - front.row;
    if (deleteCount > 0) {
        m_textBuffer[front.row].erase(front.col);
        m_textBuffer[back.row].erase(0, back.col);
        m_textBuffer[front.row].insert(m_textBuffer[front.row].size(), m_textBuffer[back.row]);
        m_textBuffer.erase(m_textBuffer.begin() + front.row + 1, m_textBuffer.begin() + back.row + 1);
    } else {
        m_textBuffer[front.row].erase(front.col, back.col - front.col);
    }
    m_cursorState.pos = front;
    m_cursorState.selectionStart = std::nullopt;
}

void Editor::backspace() {
    if (m_cursorState.selectionStart != std::nullopt) {
        deleteSelection();
        return;
    }
    if (unindent()) {
        return;
    }
    if (m_cursorState.pos.col > 0) {
        m_cursorState.pos.col -= 1;
        if (isContainer(m_textBuffer[m_cursorState.pos.row][m_cursorState.pos.col]) &&
            m_textBuffer[m_cursorState.pos.row][m_cursorState.pos.col + 1] == correspondingContainer(m_textBuffer[m_cursorState.pos.row][m_cursorState.pos.col])) {
            m_textBuffer[m_cursorState.pos.row].erase(m_cursorState.pos.col, 2);
        } else {
            m_textBuffer[m_cursorState.pos.row].erase(m_textBuffer[m_cursorState.pos.row].begin() + m_cursorState.pos.col);
        }
    } else if (m_cursorState.pos.row > 0) {
        m_cursorState.pos.col = m_textBuffer[m_cursorState.pos.row - 1].size();
        m_textBuffer[m_cursorState.pos.row - 1] += m_textBuffer[m_cursorState.pos.row];
        m_textBuffer.erase(m_textBuffer.begin() + m_cursorState.pos.row);
        m_cursorState.pos.row -= 1;
    }
}

void Editor::cursorLeft() {
    if (m_cursorState.selectionStart == std::nullopt) {
        if (m_cursorState.pos.col == 0) {
            if (m_cursorState.pos.row > 0) {
                m_cursorState.pos.row = qMax(m_cursorState.pos.row - 1, (unsigned int)0);
                m_cursorState.pos.col = m_textBuffer[m_cursorState.pos.row].length();
            }
        } else {
            m_cursorState.pos.col -= 1;
        }
    } else {
        auto [front, _] = getSelectionEnds();
        m_cursorState.pos = front;
        m_cursorState.selectionStart = std::nullopt;
    }
}

void Editor::cursorRight() {
    if (m_cursorState.selectionStart == std::nullopt) {
        if (m_cursorState.pos.col >= m_textBuffer[m_cursorState.pos.row].length()) {
            if (m_cursorState.pos.row < m_textBuffer.size() - 1) {
                m_cursorState.pos.row += 1;
                m_cursorState.pos.col = 0;
            }
        } else {
            m_cursorState.pos.col += 1;
        }
    } else {
        auto [_, back] = getSelectionEnds();
        m_cursorState.pos = back;
        m_cursorState.selectionStart = std::nullopt;
    }
}

void Editor::cursorUp() {
    if (m_cursorState.selectionStart != std::nullopt) {
        auto [front, _] = getSelectionEnds();
        m_cursorState.pos = front;
        m_cursorState.selectionStart = std::nullopt;
    }
    if (m_cursorState.pos.row == 0) {
        m_cursorState.pos.col = 0;
    } else {
        m_cursorState.pos.row -= 1;
        m_cursorState.pos.col = qMin(m_textBuffer[m_cursorState.pos.row].length(), m_cursorState.pos.col);
    }
}

void Editor::cursorDown() {
    if (m_cursorState.selectionStart != std::nullopt) {
        auto [_, back] = getSelectionEnds();
        m_cursorState.pos = back;
        m_cursorState.selectionStart = std::nullopt;
    }
    if (m_cursorState.pos.row >= m_textBuffer.size() - 1) {
        m_cursorState.pos.col = m_textBuffer.back().size();
    } else {
        m_cursorState.pos.row += 1;
        m_cursorState.pos.col = qMin(m_textBuffer[m_cursorState.pos.row].length(), m_cursorState.pos.col);
    }
}

void Editor::selectAll() {
    m_cursorState.selectionStart = { 0, 0 };
    m_cursorState.pos = { static_cast<unsigned int>(m_textBuffer.size() - 1), static_cast<unsigned int>(m_textBuffer.back().size()) };
}

void Editor::copy() {
    QClipboard* c = QGuiApplication::clipboard();

    QString content;
    auto [front, back] = getSelectionEnds();
    for (std::size_t r = front.row; r <= back.row; r++) {
        if (r == front.row) {
            content += QString::fromStdString((m_textBuffer[r].substr(front.col)));
            if (front.row != back.row) {
                content += "\n";
            }
        } else if (r == back.row) {
            content += QString::fromStdString((m_textBuffer[r].substr(0, back.col)));
        } else {
            content += QString::fromStdString(m_textBuffer[r]) + "\n";
        }
    }

    c->setText(content);
}

void Editor::paste() {
    QClipboard* c = QGuiApplication::clipboard();

    QStringList content = c->text().split('\n');
    for (int i = 0; i < content.count(); i++) {
        std::string str = content[i].toStdString();

        if (m_cursorState.selectionStart != std::nullopt) {
            deleteSelection();
        }

        if (i != 0) {
            m_textBuffer.insert(m_textBuffer.begin() + m_cursorState.pos.row, str);
        } else {
            m_textBuffer[m_cursorState.pos.row].insert(m_cursorState.pos.col, str);
        }
        m_cursorState.pos.row += 1;
    }
    m_cursorState.pos.row -= 1;
    m_cursorState.pos.col = m_textBuffer[m_cursorState.pos.row].size();
}

void Editor::undo() {

}

void Editor::redo() {

}

void Editor::save(const QString& fileName) {
    QFile f(m_projectPath + fileName);
    f.open(QIODevice::WriteOnly);

    QTextStream t(&f);
    for (const std::string& l : m_textBuffer) {
        t << QString::fromStdString(l) << "\n";
    }

    m_currentFileChanged = false;
}

void Editor::saveTemp() {
    QFile f(m_tempPath + m_currentFileName);
    f.open(QIODevice::WriteOnly);

    QTextStream t(&f);
    for (const std::string& l : m_textBuffer) {
        t << QString::fromStdString(l) << "\n";
    }
}

void Editor::compile(const QString& fileName) {
    if (m_compileProcess->state() != QProcess::ProcessState::NotRunning) return;

    save(fileName);

    const QString path = m_projectPath + fileName;
    m_compileProcess->start("javac", QStringList() << "-classpath" << m_projectPath << path);
}

void Editor::compilePotentiallyClosed(const QString& fileName) {
    if (m_compileProcess->state() != QProcess::ProcessState::NotRunning) return;
    for (int i = 0; i < m_fileTabBar->count(); i++) {
        if (m_fileTabBar->tabText(i) == fileName) {
            compile(fileName);
            return;
        }
    }

    const QString path = m_tempPath + fileName;
    m_compileProcess->start("javac", QStringList() << "-classpath" << m_projectPath << path);
}

void Editor::compileFinished() {
    emit linted(m_currentFileName.chopped(5), m_compileProcess->readAllStandardError().size() > 0, m_currentFileChanged);

    update();
}

void Editor::lint() {
    if (m_lintProcess->state() != QProcess::ProcessState::NotRunning) {
        m_wantAnotherLint = true;
        return;
    }
    m_lints.clear();
    saveTemp();
    const QString path = m_tempPath + m_currentFileName;
    m_lintProcess->start("javac", QStringList() << "-Xlint:all" << "-classpath" << m_projectPath << path);
}

void Editor::lintFinished() {
    auto l = m_lintProcess->readAllStandardError().split('\n');
    if (l.length() == 0 || (l.length() == 1 && l[0] == "")) {
        m_lints.clear();
        emit linted(m_currentFileName.chopped(5), false, m_currentFileChanged);
    } else {
        int count = l[l.count() - 2].split(' ')[0].toInt();
        int gap = 3;
        for (int i = 0; i < count; i++) {
            if (l[i * gap].split(':').size() < 4) {
                for (int j = 4; j < 8; j++)  { // 8 is arbitrary
                    if (l[i * j].split(':').size() >= 4) {
                        gap = j;
                        break;
                    }
                }
            }
            QString e = l[i * gap].split(':').back().trimmed();
            unsigned int row = l[i * gap].split(':')[1].toInt() - 1;
            unsigned int col = qMin(static_cast<unsigned int>(l[i * gap + 2].size() - 1), static_cast<unsigned int>(m_textBuffer[row].size() - 1));
            m_lints.push_back(LintInfo {
                .error = e,
                .pos = { row, col }
            });
        }
        emit linted(m_currentFileName.chopped(5), true, m_currentFileChanged);
    }

    if (m_wantAnotherLint) {
        lint();
        m_wantAnotherLint = false;
    }

    update();
}

void Editor::loadFile(const QString& path) {
    QFile f(path);
    f.open(QIODevice::ReadOnly | QIODevice::Text);

    m_textBuffer = {};

    QTextStream ts(&f);
    QString l = ts.readLine();
    while (!l.isNull()) {
        m_textBuffer.push_back(l.toStdString());
        l = ts.readLine();
    }

    m_cursorState.pos.row = 0;
    m_cursorState.pos.col = 0;
    m_cursorState.selectionStart = std::nullopt;
    m_renderData.scroll = 0.0f;
    m_lints.clear();

    lint();
    update();
}

void Editor::openClassFromGUI(const QString& path) {
    if (m_fileTabBar->tabText(0) == "untitled") {
        m_fileTabBar->removeTab(0);
    }

    const QString fileName = path.split("/").back();

    for (int i = 0; i < m_fileTabBar->count(); i++) {
        if (m_fileTabBar->tabText(i) == fileName) {
            m_fileTabBar->setCurrentIndex(i);
            return;
        }
    }
    m_fileTabBar->addTab(fileName);
    m_fileTabBar->setTabData(m_fileTabBar->count() - 1, path);
    m_fileTabBar->setCurrentIndex(m_fileTabBar->count() - 1);
    m_fileTabBar->show();

    m_bufferCache[m_currentFileName] = { m_textBuffer, m_cursorState, m_renderData.scroll, m_currentFileChanged };

    m_currentFileName = path.split('/').back();
    m_projectPath = path.chopped(m_currentFileName.size());
    m_currentFileChanged = !fileIsSameAsTemp(m_projectPath, m_currentFileName);

    QFileInfo f(m_tempPath + fileName);
    if (f.exists()) {
        loadFile(m_tempPath + fileName);
    } else {
        loadFile(path);
    }
}

void Editor::closeClass(const QString& className) {
    for (int i = 0; i < m_fileTabBar->count(); i++) {
        if (m_fileTabBar->tabText(i) == className + ".java") {
            tabClosed(i);
            return;
        }
    }
}

float Editor::properCursorPos(unsigned int x, unsigned int y) {
    if (x == 0) return m_renderData.xOffset;

    const TextRenderData& t = m_renderData.text;
    const FontGlyph& g = m_fontData.fontGlyphs[m_textBuffer[y][x - 1]];
    return t.vertices[t.rowIndices[y] + (x - 1) * 6 + 1].x + g.ax - g.bl;
}

float Editor::maxScroll() {
    const float visibleLines = ((float)m_renderData.height - m_renderData.yOffset) / m_fontData.height;
    if (m_textBuffer.size() < visibleLines) {
        return 0.0f;
    }
    return (int)((float)m_textBuffer.size() - visibleLines) * m_fontData.height + 20.0f;
}

Coord Editor::mouseToCursorPos(unsigned int x, unsigned int y) {
    Coord r;
    r.col = 0;
    r.row = ((float)y + m_renderData.scroll - m_renderData.yOffset - (m_fontData.height / 2)) / m_fontData.height;
    r.row = qMin(m_textBuffer.size() - 1, r.row);

    for (std::size_t i = 0; i < m_textBuffer[r.row].length(); i++) {
        if (x >= m_renderData.text.vertices[m_renderData.text.rowIndices[r.row] + (i * 6) + 1].x + (m_fontData.fontGlyphs[m_textBuffer[r.row][i]].ax / 2.0f)) {
            r.col++;
        } else {
            break;
        }
    }

    return r;
}

std::tuple<Coord, Coord> Editor::getSelectionEnds() {
    if (m_cursorState.selectionStart == std::nullopt) {
        return { m_cursorState.pos, m_cursorState.pos };
    }
    Coord front = *m_cursorState.selectionStart;
    Coord back = m_cursorState.pos;
    if (front.row > back.row || (front.row == back.row && front.col > back.col)) {
        front = m_cursorState.pos;
        back = *m_cursorState.selectionStart;
    }
    return { front, back };
}

QRect Editor::getLintHoverRegion(int index) {
    int startX = properCursorPos(m_lints[index].pos.col, m_lints[index].pos.row);
    int endX = properCursorPos(m_lints[index].pos.col + 1, m_lints[index].pos.row);
    int y = m_fontData.height * m_lints[index].pos.row + m_renderData.yOffset - m_renderData.scroll;
    return QRect(startX - 10, y - 10, endX - startX + 20, m_fontData.height + 20);
}

bool Editor::fileIsSameAsTemp(const QString& projectPath, const QString& fileName) const {
    QFile tempFile(QDir::cleanPath(m_tempPath + "/" + fileName));
    QFile realFile(QDir::cleanPath(projectPath + "/" + fileName));
    if (!tempFile.exists() || !realFile.exists()) {
        return false;
    }
    tempFile.open(QFile::ReadOnly);
    realFile.open(QFile::ReadOnly);

    return tempFile.readAll() == realFile.readAll();
}

void Editor::loadFontGlyphs() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("Failed to initialize FreeType");
    }

    std::string path = QCoreApplication::applicationDirPath().toStdString() + "/res/fonts/RobotoMono-Regular.ttf";
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        throw std::runtime_error("Failed to load font face");
    }

    FT_Set_Pixel_Sizes(face, 0, m_fontData.size);

    unsigned int rowWidth = 0, rowHeight = 0;
    m_fontData.atlasWidth = 0, m_fontData.atlasHeight = 0;

    unsigned int gIndex;
    unsigned int charCode = FT_Get_First_Char(face, &gIndex);
    while (gIndex != 0) {
        if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
            qDebug() << "Failed to load glyph\n";
            charCode = FT_Get_Next_Char(face, charCode, &gIndex);
            continue;
        }

        if (rowWidth + face->glyph->bitmap.width + 1 >= GL_MAX_TEXTURE_SIZE) {
            m_fontData.atlasWidth = std::max(m_fontData.atlasWidth, rowWidth);
            m_fontData.atlasHeight += rowHeight;
            rowWidth = 0;
            rowHeight = 0;
        }
        rowWidth += face->glyph->bitmap.width + 1;
        rowHeight = std::max(rowHeight, face->glyph->bitmap.rows);

        charCode = FT_Get_Next_Char(face, charCode, &gIndex);
    }

    m_fontData.atlasWidth = std::max(m_fontData.atlasWidth, rowWidth);
    m_fontData.atlasHeight += rowHeight;

    glBindTexture(GL_TEXTURE_2D, m_renderData.text.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_fontData.atlasWidth, m_fontData.atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int x = 0, y = 0;
    rowHeight = 0;

    m_fontData.fontGlyphs.clear();

    charCode = FT_Get_First_Char(face, &gIndex);
    while (gIndex != 0) {
        if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
            charCode = FT_Get_Next_Char(face, charCode, &gIndex);
            continue;
        }

        if (x + face->glyph->bitmap.width + 1 >= GL_MAX_TEXTURE_SIZE) {
            y += rowHeight;
            rowHeight = 0;
            x = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        m_fontData.fontGlyphs.emplace(charCode, FontGlyph {
            static_cast<float>(face->glyph->advance.x >> 6), static_cast<float>(face->glyph->advance.y >> 6),
            static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows),
            static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top),
            static_cast<float>(x) / static_cast<float>(m_fontData.atlasWidth),
            static_cast<float>(y) / static_cast<float>(m_fontData.atlasHeight)
        });

        rowHeight = std::max(rowHeight, face->glyph->bitmap.rows);
        x += face->glyph->bitmap.width + 1;

        charCode = FT_Get_Next_Char(face, charCode, &gIndex);
    }

    m_fontData.height = face->size->metrics.height >> 6;
    m_renderData.cursor.height = 2 * m_fontData.height - static_cast<float>(m_fontData.size);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

std::vector<TextVertex> Editor::generateBatchedVertices(const std::vector<std::string>& textContent) {
    std::vector<TextVertex> v;

    float y = m_fontData.height + m_renderData.yOffset;

    m_renderData.text.rowIndices.clear();

    bool multiLineCommentActive = false;

    for (unsigned int r = 0; r < textContent.size(); r++) {
        m_renderData.text.rowIndices.push_back(v.size());

        float x = m_renderData.xOffset;

        std::string currentWord;
        int cwPos = -1;

        bool commentActive = false;
        bool quoteActive = false;

        for (unsigned int c = 0; c < textContent[r].length(); c++) {
            const auto& glyph = m_fontData.fontGlyphs[textContent[r][c]];

            const float xPos = x + glyph.bl;
            const float yPos = y - glyph.bt;

            const float w = glyph.bw;
            const float h = glyph.bh;

            const float tw = glyph.bw / static_cast<float>(m_fontData.atlasWidth);
            const float th = glyph.bh / static_cast<float>(m_fontData.atlasHeight);


            char l = textContent[r][c];

            float red = 1.0f;
            float green = 1.0f;
            float blue = 1.0f;
            if (l == '/' && c + 1 < textContent[r].length()) {
                if (textContent[r][c + 1] == '/') {
                    commentActive = true;
                } else if (textContent[r][c + 1] == '*' && !commentActive) {
                    multiLineCommentActive = true;
                }
            }
            if (commentActive) {
                red = 0.5f;
                green = 0.5f;
                blue = 0.5f;
            } else if (multiLineCommentActive) {
                if (l == '/' && c > 0 && textContent[r][c - 1] == '*') {
                    multiLineCommentActive = false;
                }
                red = 0.5f;
                green = 0.5f;
                blue = 0.5f;
            } else if (l == '\'' || l == '\"') {
                if (quoteActive) {
                    quoteActive = false;
                } else {
                    quoteActive = true;
                }
                red = 1.0f;
                green = 0.286f;
                blue = 0.62f;
            } else if (quoteActive) {
                red = 0.824f;
                green = 0.392f;
                blue = 0.714f;
            } else if (l == ' ' || l == ';' || l == '(' || l == ')' || l == '.' || l == '[' || l == ']') {
                currentWord = "";
                cwPos = -1;
            } else if (l == '+' || l == '-' || l == '=' || l == '%' || l == '<') {
                red = 0.467f;
                green = 0.608f;
                blue = 0.906f;
                currentWord = "";
                cwPos = -1;
            } else {
                currentWord += l;
                if (cwPos == -1) cwPos = c;
            }
            if (currentWord.length() > 0 && currentWord[0] == 'S') {
                red = 1.0f;
                green = 0.286f;
                blue = 0.62f;
            } else if (currentWord == "int" || currentWord == "float" || currentWord == "boolean" || currentWord == "char" || currentWord == "void") {
                red = 0.286f;
                green = 0.714f;
                blue = 1.0f;
            } else if (currentWord == "class" || currentWord == "public" || currentWord == "private" || currentWord == "static" || currentWord == "if" || currentWord == "for" || currentWord == "else" || currentWord == "while" || currentWord == "new") {
                red = 0.824f;
                green = 0.392f;
                blue = 0.714f;
            }
            if (currentWord.find_first_not_of("0123456789") == std::string::npos && currentWord.length() > 0) {
                red = 0.643f;
                green = 0.502f;
                blue = 0.812f;
            }
            if (currentWord.length() > 0 && cwPos != -1) {
                for (std::size_t i = 0; i < currentWord.length() - 1; i++) {
                    for (int j = 0; j < 6; j++) {
                        const int idx = m_renderData.text.rowIndices[r] + (cwPos + i) * 6 + j;
                        v[idx].r = red;
                        v[idx].g = green;
                        v[idx].b = blue;
                    }
                }
            }

            v.insert(v.end(), {
                { xPos + w, yPos, glyph.tx + tw, glyph.ty, red, green, blue },
                { xPos, yPos, glyph.tx, glyph.ty, red, green, blue },
                { xPos, yPos + h, glyph.tx, glyph.ty + th, red, green, blue },
                { xPos, yPos + h, glyph.tx, glyph.ty + th, red, green, blue },
                { xPos + w, yPos + h, glyph.tx + tw, glyph.ty + th, red, green, blue },
                { xPos + w, yPos, glyph.tx + tw, glyph.ty, red, green, blue }
            });

            x += glyph.ax;
            y += glyph.ay;
        }
        y += m_fontData.height;
    }

    return v;
}

std::vector<TextVertex> Editor::generateLineNumberVertices() {
    std::vector<TextVertex> v;

    float y = m_fontData.height + m_renderData.yOffset;

    for (std::size_t i = 1; i <= m_textBuffer.size(); i++) {
        const std::string s = std::to_string(i);

        float x = 30.0f;

        for (int j = s.length() - 1; j >= 0; j--) {
            const auto& glyph = m_fontData.fontGlyphs[s[j]];

            const float xPos = x + glyph.bl;
            const float yPos = y - glyph.bt;

            const float w = glyph.bw;
            const float h = glyph.bh;

            const float tw = glyph.bw / static_cast<float>(m_fontData.atlasWidth);
            const float th = glyph.bh / static_cast<float>(m_fontData.atlasHeight);

            v.insert(v.end(), {
                { xPos + w, yPos, glyph.tx + tw, glyph.ty, 0.8f, 0.8f, 0.8f },
                { xPos, yPos, glyph.tx, glyph.ty, 0.8f, 0.8f, 0.8f },
                { xPos, yPos + h, glyph.tx, glyph.ty + th, 0.8f, 0.8f, 0.8f },
                { xPos, yPos + h, glyph.tx, glyph.ty + th, 0.8f, 0.8f, 0.8f },
                { xPos + w, yPos + h, glyph.tx + tw, glyph.ty + th, 0.8f, 0.8f, 0.8f },
                { xPos + w, yPos, glyph.tx + tw, glyph.ty, 0.8f, 0.8f, 0.8f }
            });

            if (j > 0) {
                x -= m_fontData.fontGlyphs[s[j - 1]].ax;
            }
        }
        y += m_fontData.height;
    }
    return v;
}

QString Editor::getTempPath() const {
    return m_tempPath;
}

void Editor::tabChanged(int tabIndex) {
    m_bufferCache[m_currentFileName] = { m_textBuffer, m_cursorState, m_renderData.scroll, m_currentFileChanged };

    const auto& e = m_bufferCache[m_fileTabBar->tabText(tabIndex)];
    m_textBuffer = e.textBuffer;
    m_cursorState = e.cursorState;
    m_renderData.scroll = e.scroll;
    m_currentFileChanged = e.currentFileChanged;
    m_currentFileName = m_fileTabBar->tabText(tabIndex);
    m_lints.clear();

    update();
}

void Editor::tabClosed(int tabIndex) {
    m_fileTabBar->removeTab(tabIndex);

    m_bufferCache.erase(m_currentFileName);

    const auto& e = m_bufferCache[m_fileTabBar->tabText(m_fileTabBar->currentIndex())];
    m_textBuffer = e.textBuffer;
    m_cursorState = e.cursorState;
    m_renderData.scroll = e.scroll;
    m_currentFileChanged = e.currentFileChanged;
    m_currentFileName = m_fileTabBar->tabText(m_fileTabBar->currentIndex());
    m_lints.clear();

    if (m_fileTabBar->count() == 0) {
        m_fileTabBar->addTab("untitled");
        m_textBuffer = {""};
    }

    update();
}
