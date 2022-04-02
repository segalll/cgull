#include "project.h"

#include <QFile>
#include <QTextStream>

Project::Project()
{

}

void Project::createClass(const std::string &className)
{
    addClass(className);
    writeDefaultClass(classes.back());
}

void Project::addClass(const std::string &className)
{
    classes.emplace_back(className, path);
}

void Project::writeDefaultClass(Class &c)
{
    std::string defaultContent = "/**\n"
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

    QTextStream out(&classes.back().file);
    out << defaultContent.c_str();
}

void Project::create(const std::string &projectName, const std::string &projectPath)
{
    name = projectName;
    path = projectPath;
}

std::string Project::getPath() const
{
    return path;
}
