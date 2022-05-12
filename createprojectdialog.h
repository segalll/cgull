#ifndef CREATEPROJECTDIALOG_H
#define CREATEPROJECTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

class CreateProjectDialog : public QDialog
{
public:
    CreateProjectDialog();
    QString getPath();
private slots:
    void browse();
    void changePath(const QString& newText);
private:
    QLineEdit* m_nameInput;
    QLineEdit* m_locationInput;
    QPushButton* m_browseButton;
    QLabel* m_path;
};

#endif // CREATEPROJECTDIALOG_H
