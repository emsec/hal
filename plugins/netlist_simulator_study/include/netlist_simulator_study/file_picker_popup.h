#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

namespace hal
{
    class FilePickerPopup : public QDialog
    {
        Q_OBJECT

    public:
        explicit FilePickerPopup(QWidget* parent = nullptr);
        std::string get_file_path();

    private:
        QLineEdit* filePathLineEdit;
        QPushButton* openFileButton;
        QPushButton* okButton;

        void openFileDialog();
        void acceptSelection();
    };
}    // namespace hal
