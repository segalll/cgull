#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "classdiagram.h"
#include "createclassdialog.h"
#include "createprojectdialog.h"
#include "project.h"

#include <QDir>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);

    m_scene = new QGraphicsScene();
    m_scene->setSceneRect(0, 0, 100, 50); // random numbers work for this idk why
    m_ui->graphicsView->setScene(m_scene);
    m_ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_ui->newClassButton, &QPushButton::clicked, this, &MainWindow::createClass);
    connect(m_ui->compileButton, &QPushButton::clicked, this, &MainWindow::compile);

    m_classDiagramEmitter = new ClassDiagramEmitter;
    connect(m_classDiagramEmitter, &ClassDiagramEmitter::classOpened, this, &MainWindow::openClass);
    connect(m_classDiagramEmitter, &ClassDiagramEmitter::classClosed, this, &MainWindow::closeClass);
    connect(m_classDiagramEmitter, &ClassDiagramEmitter::classMoved, this, &MainWindow::save);

    m_editor = new Editor;

    connect(m_editor, &Editor::linted, this, &MainWindow::classStateChanged);

    m_runner = new Runner(m_currentProject.getPath());

    loadProject();

    m_createProjectDialog = new CreateProjectDialog;
    connect(m_createProjectDialog, &CreateProjectDialog::accepted, this, &MainWindow::projectCreated);

    QMenu* m = menuBar()->addMenu("File");
    m->addAction("New Project...", [this]() {
        m_createProjectDialog->exec();
    });
    m->addAction("Open Project...", [this]() {
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setNameFilter("CGull project files (*.cgull *.bluej)");
        QString path;
        if (dialog.exec() && dialog.selectedFiles().size() > 0) {
            path = dialog.selectedFiles().front();
        } else {
            return;
        }
        path = path.chopped(QString("package.cgull").count());
        m_currentProject = Project(path);
        m_scene->clear();
        loadProject();
    });
    m->addAction("Save", this, SLOT(save()));
    m->addAction("Save As...");
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::loadProject() {
    QDir projectDir(m_currentProject.getPath());
    QStringList classes = projectDir.entryList(QStringList() << "*.java", QDir::Files);
    foreach (QString filename, classes) {
        const QString className = filename.split(".java")[0];
        m_currentProject.addClass(className);
        m_scene->addItem(new ClassEntry(className, QDir::cleanPath(m_currentProject.getPath() + "/" + filename), m_classDiagramEmitter, m_runner));
    }
    QMap<QString, QPoint> classPositions = m_currentProject.getClassPositions();
    for (QGraphicsItem* c : m_scene->items()) {
        if (classPositions.contains(c->data(0).toString())) {
            c->setPos(classPositions[c->data(0).toString()]);
        }
    }
}

void MainWindow::createClass() {
    CreateClassDialog* dialog = new CreateClassDialog;

    if (dialog->exec() == QDialog::Accepted) {
        ClassEntry* newClass = new ClassEntry(dialog->getClassName(), m_currentProject.getPath() + dialog->getClassName() + ".java", m_classDiagramEmitter, m_runner);
        m_scene->addItem(newClass);
        m_currentProject.createClass(dialog->getClassName());
    }
}

void MainWindow::openClass(QString classPath) {
    if (!m_editor->isVisible()) {
        m_editor->show();
    }
    m_editor->openClassFromGUI(classPath);
}

void MainWindow::closeClass(QString className) {
    m_editor->closeClass(className);
}

void MainWindow::compile() {
    // this is a little hacky, maybe add class list somewhere
    for (QGraphicsItem* c : m_scene->items()) { // means of iterating over class names
        m_editor->compilePotentiallyClosed(c->data(0).toString() + ".java");
    }
}

void MainWindow::save() {
    QString serialization;
    for (const QGraphicsItem* p : m_scene->items()) {
        serialization += "--" + p->data(0).toString() + "\n"; // data(0) is class name
        QString s = QString::number(p->pos().x()) + " " + QString::number(p->pos().y());
        serialization += s + "\n";
    }
    m_currentProject.writeProjectFile(serialization);
}

void MainWindow::projectCreated() {
    m_currentProject.create(m_createProjectDialog->getPath());
    m_scene->clear();
}

void MainWindow::classStateChanged(QString className, bool errors, bool compiled) {
    for (QGraphicsItem* p : m_scene->items()) {
        if (p->data(0).toString() == className) {
            p->setData(1, errors);
            p->setData(2, compiled);
            p->update();
            return;
        }
    }
}
