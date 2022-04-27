#ifndef RUNNER_H
#define RUNNER_H

#include <QPlainTextEdit>
#include <QProcess>

class Runner : public QPlainTextEdit
{
    Q_OBJECT
public:
    Runner(QString projectPath);

    void run(QString fileName);

private slots:
    void textOutputted();

private:
    QString projectPath;
    QProcess* m_runProcess;
};

#endif // RUNNER_H
