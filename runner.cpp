#include "runner.h"

Runner::Runner(QString projectPath) : projectPath(projectPath) {
    setReadOnly(true);

    m_runProcess = new QProcess(this);
    m_runProcess->setEnvironment(QProcess::systemEnvironment());

    connect(m_runProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(textOutputted()));
}

void Runner::run(QString fileName) {
    if (m_runProcess->state() != QProcess::ProcessState::NotRunning) return;

    m_runProcess->start("java", QStringList() << "-classpath" << projectPath << fileName);
}

void Runner::textOutputted() {
    setPlainText(m_runProcess->readAllStandardOutput());
}
