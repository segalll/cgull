#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "classdiagram.h"
#include "createprojectdialog.h"
#include "editor.h"
#include "project.h"
#include "runner.h"

#include <QGraphicsScene>
#include <QMainWindow>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void loadProject();
    void detectErrors(const QString& fileName);

private slots:
    void createClass();
    void openClass(QString classPath);
    void closeClass(QString className);
    void compile();
    void save();
    void projectCreated();
    void classStateChanged(QString className, bool errors, bool changed);
    void errorDetectionFinished();

private:
    Ui::MainWindow* m_ui;
    QGraphicsScene* m_scene;
    ClassDiagramEmitter* m_classDiagramEmitter;
    Editor* m_editor;
    Runner* m_runner;
    Project m_currentProject;
    CreateProjectDialog* m_createProjectDialog;
    QProcess* m_errorDetectionProcess;
    QString m_currentErrorDetectionClass;
    QQueue<QString> m_errorDetectionQueue;
};
#endif // MAINWINDOW_H
