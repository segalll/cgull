#ifndef PROJECT_H
#define PROJECT_H

#include <QFile>

#include <list>
#include <memory>
#include <vector>

struct Class {
    QString name;
    QFile file;
    std::vector<std::unique_ptr<Class>> dependencies;

    Class(QString name, QString projectPath) : name(name) {
        file.setFileName(projectPath + "/" + name + ".java");
        file.open(QIODevice::ReadWrite | QIODevice::Text);
    }
};

class Project
{
public:
    Project();
    void createClass(const QString& className);
    void addClass(const QString& className);
    void writeDefaultClass(Class& c);
    void create(const QString& projectName, const QString& projectPath);
    QString getPath() const;
private:
    std::list<Class> m_classes;
    QString m_name;
    QString m_path;
};

#endif // PROJECT_H
