#include "project.h"

#include <QFile>
#include <QTextStream>

Project::Project() {}

Project::Project(const QString& projectName, const QString& projectLocation) {
    m_path = projectLocation + projectName + "/";
}

Project::Project(const QString& projectPath) {
    m_path = projectPath;
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

QString Project::getPath() const {
    return m_path;
}
