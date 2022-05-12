#include "createprojectdialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>

CreateProjectDialog::CreateProjectDialog() {
    QFormLayout* layout = new QFormLayout(this);
    layout->setLabelAlignment(Qt::AlignLeft);

    QLabel* nameLabel = new QLabel(QString("Name: "));
    m_nameInput = new QLineEdit(this);
    m_nameInput->setMinimumSize(150, 25);
    m_nameInput->setTextMargins(2, 0, 2, 0);

    connect(m_nameInput, &QLineEdit::textChanged, this, &CreateProjectDialog::changePath);

    layout->addRow(nameLabel, m_nameInput);

    QHBoxLayout* locationLayout = new QHBoxLayout;
    QLabel* locationLabel = new QLabel(QString("Location: "));

    m_locationInput = new QLineEdit(this);
    m_locationInput->setMinimumSize(150, 25);
    m_locationInput->setTextMargins(2, 0, 2, 0);

    connect(m_locationInput, &QLineEdit::textChanged, this, &CreateProjectDialog::changePath);

    m_browseButton = new QPushButton(this);
    m_browseButton->setText("Browse...");

    connect(m_browseButton, &QPushButton::clicked, this, &CreateProjectDialog::browse);

    locationLayout->addWidget(m_locationInput);
    locationLayout->addWidget(m_browseButton);

    layout->addRow(locationLabel, locationLayout);

    QLabel* pathLabel = new QLabel(QString("Path: "));
    m_path = new QLabel(QString("/"));

    layout->addRow(pathLabel, m_path);

    QDialogButtonBox* confirmButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(confirmButtons);

    connect(confirmButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(confirmButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(layout);
    setMinimumSize(layout->sizeHint());
    setMaximumSize(layout->sizeHint());
}

QString CreateProjectDialog::getPath() {
    return m_path->text();
}

void CreateProjectDialog::browse() {
    QString directory = QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath());

    if (!directory.isEmpty()) {
        m_locationInput->setText(directory);
    }
}

void CreateProjectDialog::changePath(const QString& newText) {
    Q_UNUSED(newText);
    m_path->setText(QDir::cleanPath(m_locationInput->text() + QDir::separator() + m_nameInput->text()));
}
