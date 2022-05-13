#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "classdiagram.h"
#include "createprojectdialog.h"
#include "editor.h"
#include "project.h"
#include "runner.h"

#include <QGraphicsScene>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void loadProject();

private slots:
    void createClass();
    void openClass(QString classPath);
    void closeClass(QString className);
    void compile();
    void save();
    void projectCreated();

private:
    Ui::MainWindow* m_ui;
    QGraphicsScene* m_scene;
    ClassDiagramEmitter* m_classDiagramEmitter;
    Editor* m_editor;
    Runner* m_runner;
    Project m_currentProject;
    CreateProjectDialog* m_createProjectDialog;
};
#endif // MAINWINDOW_H
