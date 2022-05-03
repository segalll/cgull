#include "createclassdialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>

CreateClassDialog::CreateClassDialog(QWidget* parent) : QDialog(parent) {
    QFormLayout* layout = new QFormLayout(this);
    layout->setLabelAlignment(Qt::AlignLeft);

    QLabel* nameLabel = new QLabel(QString("Class name: "));
    m_nameInput = new QLineEdit(this);
    m_nameInput->setMinimumSize(150, 25);
    m_nameInput->setTextMargins(2, 0, 2, 0);
    layout->addRow(nameLabel, m_nameInput);

    QString typeOptionNames[] = { "Class", "Abstract Class", "Interface", "Unit Test", "Enum", "JavaFX Class" };

    QLabel* typeLabel = new QLabel(QString("Class type:"));
    m_typeComboBox = new QComboBox;
    for (QString name: typeOptionNames) {
        m_typeComboBox->addItem(name);
    }
    layout->addRow(typeLabel, m_typeComboBox);

    QDialogButtonBox* confirmButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(confirmButtons);

    connect(confirmButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(confirmButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
    setMinimumSize(layout->sizeHint());
    setMaximumSize(layout->sizeHint());
}

QString CreateClassDialog::getClassName() const {
    return m_nameInput->text();
}

QString CreateClassDialog::getClassType() const {
    return m_typeComboBox->currentText();
}
