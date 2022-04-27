#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "createclassdialog.h"
#include "classdiagram.h"
#include "project.h"

#include <QDir>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 100, 50); // random numbers work for this idk why
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    classDiagramEmitter = new ClassDiagramEmitter;
    QObject::connect(classDiagramEmitter, &ClassDiagramEmitter::classOpened, this, &MainWindow::on_classOpened);

    currentProject.create("testing", "/Users/hsegal/Documents/testing");

    runner = new Runner("/Users/hsegal/Documents/testing/");

    QDir projectDir(currentProject.getPath().c_str());
    QStringList classes = projectDir.entryList(QStringList() << "*.java", QDir::Files);
    foreach (QString filename, classes)
    {
        const std::string className = filename.split(".java")[0].toStdString();
        currentProject.addClass(className);
        scene->addItem(new ClassEntry(className.c_str(), "/Users/hsegal/Documents/testing/" + filename, classDiagramEmitter, runner));
    }

    QMenu* m = menuBar()->addMenu("File");
    m->addAction("Open Project...");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_newClassButton_clicked()
{
    CreateClassDialog* dialog = new CreateClassDialog;

    if (dialog->exec() == QDialog::Accepted) {
        ClassEntry *newClass = new ClassEntry(dialog->getClassName(), "/Users/hsegal/Documents/testing/" + dialog->getClassName() + ".java", classDiagramEmitter, runner);
        scene->addItem(newClass);
        currentProject.createClass(dialog->getClassName().toStdString());
    }
}

void MainWindow::on_classOpened(QString classPath)
{
    if (editor == nullptr) {
        editor = new Editor;
    }
    if (!editor->isVisible()) {
        editor->show();
    }
    editor->openClassFromGUI(classPath);
}
