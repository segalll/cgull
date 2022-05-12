#include "openprojectdialog.h"

OpenProjectDialog::OpenProjectDialog() {
    setFileMode(QFileDialog::ExistingFile);
    setNameFilter("CGull project files (*.cgull *.bluej)");
}
