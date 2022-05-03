#ifndef CREATECLASSDIALOG_H
#define CREATECLASSDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class CreateClassDialog : public QDialog {
public:
    explicit CreateClassDialog(QWidget* parent = nullptr);
    QString getClassName() const;
    QString getClassType() const;
private:
    QLineEdit* m_nameInput;
    QComboBox* m_typeComboBox;
};

#endif // CREATECLASSDIALOG_H
