#ifndef EDITOR_H
#define EDITOR_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QProcess>
#include <QTabBar>

#include <unordered_map>

struct FontGlyph {
    float ax, ay; // advance.x, advance.y
    float bw, bh; // bitmap.widths, bitmap.rows
    float bl, bt; // bitmap.left, bitmap.top
    float tx, ty; // texture coord x, texture coord y
};

struct FontData {
    uint32_t size = 16;
    float height; // full height of row
    uint32_t atlasWidth, atlasHeight;
    std::unordered_map<char, FontGlyph> fontGlyphs;
};

struct TextVertex {
    float x, y;
    float tx, ty;
    float r, g, b;
};

struct TextRenderData {
    QOpenGLVertexArrayObject vao;
    std::vector<TextVertex> vertices;
    std::vector<uint32_t> rowIndices;
    QOpenGLBuffer vbo;
    QOpenGLShaderProgram shader;
    GLuint texture;
};

struct CursorRenderData {
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QOpenGLShaderProgram shader;
    float x, y;
    float height;
};

struct RenderData {
    TextRenderData text;
    CursorRenderData cursor;
    GLuint ubo;
    QMatrix4x4 proj;
    float scroll;
    unsigned int width;
    unsigned int height;
    float xOffset;
    float yOffset;
};

struct Coord {
    unsigned int row = 0;
    unsigned int col = 0;
};

inline bool operator==(const Coord& a, const Coord& b) {
    return a.row == b.row && a.col == b.col;
}

inline bool operator!=(const Coord& a, const Coord& b) {
    return a.row != b.row || a.col != b.col;
}

struct CursorState {
    Coord pos;
    std::optional<Coord> selectionStart;
    bool mouseDown = false;
};

struct LintInfo {
    QString error;
    Coord pos;
};

struct EditorState {
    std::vector<std::string> textBuffer;
    CursorState cursorState;
    float scroll;
    bool currentFileCompiles;
};

class Editor : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    Editor();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    bool event(QEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void mouseDoubleClickEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;

    void renderText();
    void renderError();
    void renderSelection();
    void renderCursor();
    void renderLineNumbers();

    void enterStr(const std::string& str);
    void newLine();
    void indent();
    bool unindent();
    void deleteSelection();
    void backspace();
    void cursorLeft();
    void cursorRight();
    void cursorUp();
    void cursorDown();
    void selectAll();
    void copy();
    void paste();
    void undo();
    void redo();

    void save(QString fileName);
    void saveTemp();
    void compile(QString fileName);
    void compilePotentiallyClosed(QString fileName); // compile normally if file is open and from temp if file closed
    void lint();

    void loadFile(QString path);
    void openClassFromGUI(QString path);
    void closeClass(QString className);

    constexpr float properCursorPos(unsigned int x, unsigned int y);
    constexpr float maxScroll();
    constexpr Coord mouseToCursorPos(unsigned int x, unsigned int y);
    constexpr std::tuple<Coord, Coord> getSelectionEnds();
    constexpr QRect getLintHoverRegion(int index);

    void loadFontGlyphs();
    std::vector<TextVertex> generateBatchedVertices(const std::vector<std::string>& textContent);
    std::vector<TextVertex> generateLineNumberVertices();

private slots:
    void tabChanged(int tabIndex);
    void tabClosed(int tabIndex);
    void compileFinished();
    void lintFinished();

private:
    std::vector<std::string> m_textBuffer;
    FontData m_fontData;
    RenderData m_renderData;
    CursorState m_cursorState;
    QTabBar* m_fileTabBar;
    QProcess* m_compileProcess;
    QProcess* m_lintProcess;
    std::vector<LintInfo> m_lints;
    QString m_projectPath;
    QString m_tempPath;
    QString m_currentFileName;
    std::map<QString, EditorState> m_bufferCache;
    bool m_wantAnotherLint = false;
    bool m_currentFileCompiles = false;
};

#endif // EDITOR_H
