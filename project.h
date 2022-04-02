#ifndef PROJECT_H
#define PROJECT_H

#include <QFile>
#include <vector>
#include <list>
#include <string>
#include <memory>

struct Class {
    std::string name;
    QFile file;
    std::vector<std::unique_ptr<Class>> dependencies;

    Class(std::string name, std::string projectPath) : name(name) {
        file.setFileName((projectPath + "/" + name + ".java").c_str());
        file.open(QIODevice::ReadWrite | QIODevice::Text);
    }
};

class Project
{
public:
    Project();
    void createClass(const std::string &className);
    void addClass(const std::string &className);
    void writeDefaultClass(Class &c);
    void create(const std::string &projectName, const std::string &projectPath);
    std::string getPath() const;
private:
    std::list<Class> classes;
    std::string name;
    std::string path;

};

#endif // PROJECT_H
