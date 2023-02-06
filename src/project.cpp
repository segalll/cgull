#include "project.h"

#include <QDir>
#include <QFile>
#include <QPoint>
#include <QTextStream>

Project::Project() {}

Project::Project(const QString& projectPath) {
    m_path = QDir::cleanPath(projectPath);
}

void Project::createClass(const QString& className) {
    addClass(className);
    writeDefaultClass(m_classes.back());
}

void Project::addClass(const QString& className) {
    m_classes.emplace_back(className, m_path);
}

void Project::writeDefaultClass(Class& c) {
    QString defaultContent = "/**\n"
        " * Write a description of class " + c.name + " here.\n"
        " *\n"
        " * @author (your name)\n"
        " * @version (a version number or a date)\n"
        " */\n"
        "public class " + c.name + "\n"
        "{\n"
        "    // instance variables - replace the example below with your own\n"
        "    private int x;\n\n"
        "    /**\n"
        "     * Constructor for objects of class " + c.name + "\n"
        "     */\n"
        "    public " + c.name + "()\n"
        "    {\n"
        "        // initialize instance variables\n"
        "        x = 0;\n"
        "    }\n\n"
        "    /**\n"
        "     * An example of a method - replace this comment with your own\n"
        "     *\n"
        "     * @param  y  a sample parameter for a method\n"
        "     * @return    the sum of x and y\n"
        "     */\n"
        "    public int sampleMethod(int y)\n"
        "    {\n"
        "        // put your code here\n"
        "        return x + y;\n"
        "    }\n"
        "}";

    QTextStream out(&c.file);
    out << defaultContent;
}

void Project::create(const QString& projectPath) {
    QDir dir;
    m_path = QDir::cleanPath(projectPath);
    if (!dir.exists(m_path)) {
        dir.mkpath(m_path);
    }

    QFile f(m_path + "/package.cgull");
    f.open(QFile::WriteOnly);
}

void Project::writeProjectFile(const QString& contents) {
    QFile f(m_path + "/package.cgull");
    f.open(QFile::WriteOnly);
    f.write(contents.toUtf8());
}

QMap<QString, QPoint> Project::getClassPositions() const {
    QMap<QString, QPoint> m;

    QFile f(m_path + "/package.cgull");
    f.open(QFile::ReadOnly);
    QString currentClass;

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString l = in.readLine();
        if (l.startsWith("--")) {
            currentClass = l.last(l.count() - 2);
        } else {
            QPoint p(l.split(' ')[0].toInt(), l.split(' ')[1].toInt());
            m[currentClass] = p;
        }
    }
    return m;
}

QString Project::getPath() const {
    return m_path;
}
