#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

#include "project.h"
#include "classdiagram.h"
#include "editor.h"
#include "runner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_newClassButton_clicked();
    void on_classOpened(QString classPath);

private:
    Ui::MainWindow *ui;

    QGraphicsScene *scene;

    ClassDiagramEmitter *classDiagramEmitter;

    Editor* editor;

    Runner* runner;

    Project currentProject;
};
#endif // MAINWINDOW_H
