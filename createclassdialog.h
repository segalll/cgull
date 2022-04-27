#ifndef CREATECLASSDIALOG_H
#define CREATECLASSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class CreateClassDialog : public QDialog
{
public:
    explicit CreateClassDialog(QWidget *parent = nullptr);
    QString getClassName() const;
    QString getClassType() const;
private:
    QLineEdit *nameInput;
    QComboBox *typeComboBox;
};

#endif // CREATECLASSDIALOG_H
