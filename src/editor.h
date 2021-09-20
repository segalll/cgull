#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <string>

namespace {
    typedef uint32_t Char;

    struct Glyph {
        Char mChar;
    };

    typedef std::vector<Glyph> Line;
    typedef std::vector<Line> Lines;

    struct Coordinates {
        unsigned int mLine, mColumn;
        Coordinates() : mLine(0), mColumn(0) {}
        Coordinates(unsigned int line, unsigned int column) : mLine(line), mColumn(column) {}

        bool operator ==(const Coordinates& o) const {
            return mLine == o.mLine && mColumn == o.mColumn;
        }

        bool operator !=(const Coordinates& o) const {
            return mLine != o.mLine || mColumn != o.mColumn;
        }

        bool operator <(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine < o.mLine;
            return mColumn < o.mColumn;
        }

        bool operator >(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine > o.mLine;
            return mColumn > o.mColumn;
        }

        bool operator <=(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine < o.mLine;
            return mColumn <= o.mColumn;
        }

        bool operator >=(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine > o.mLine;
            return mColumn >= o.mColumn;
        }
    };

    struct EditorState {
        Coordinates mSelectionStart;
        Coordinates mSelectionEnd;
        Coordinates mCursorPosition;
    };
};

class Editor {
public:
    void handleTextInput(unsigned int codepoint);
    void handleSpecialKeyInput(int key, int mods);
    void handleMouseButton(int button, int mods, unsigned int xpos, unsigned int ypos);
private:
    Lines mLines;
    Coordinates mCoords;
    EditorState mState;

    Coordinates screenPosToCoordinates(unsigned int xpos, unsigned int ypos) const;
    void enterCharacter(Char c);
};