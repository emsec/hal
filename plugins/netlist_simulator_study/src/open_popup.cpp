#include "netlist_simulator_study/file_picker_popup.h"
#include "netlist_simulator_study/netlist_simulator_study.h"

#include <QApplication>
#include <QFileDialog>
#include <QVBoxLayout>

namespace hal
{
    QString FilePickerPopup::last_file_input = "";
    
    /**
     * @brief Construct a new File Picker Popup:: File Picker Popup object
     * 
     * @param parent 
     */
    FilePickerPopup::FilePickerPopup(QWidget* parent) : QDialog(parent)
    {
        openFileButton   = new QPushButton("Browse...", this);
        okButton         = new QPushButton("OK", this);
        filePathLineEdit = new QLineEdit(this);

        std::cout << "Input: " << last_file_input.toStdString() << std::endl;

        if(last_file_input != ""){
            filePathLineEdit->setText(last_file_input);
        }

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(filePathLineEdit);
        layout->addWidget(openFileButton);
        layout->addWidget(okButton);

        connect(openFileButton, &QPushButton::clicked, this, &FilePickerPopup::openFileDialog);
        connect(okButton, &QPushButton::clicked, this, &FilePickerPopup::acceptSelection);

        setWindowTitle("Choose sim input");
    }

    /**
     * @brief Function to open up the system file picker window
     * 
     */
    void FilePickerPopup::openFileDialog()
    {
        QString fileFilter = "Input Files (*.csv *.vcd *.json);;All Files (*)";
        QString filePath   = QFileDialog::getOpenFileName(this, "Select a file", "", fileFilter);
        if (!filePath.isEmpty())
        {
            filePathLineEdit->setText(filePath);
        }
    }

    /**
     * @brief Handles the event for clicking the accept button
     * 
     */
    void FilePickerPopup::acceptSelection()
    {
        if (!filePathLineEdit->text().isEmpty())
        {
            last_file_input = filePathLineEdit->text();
            accept();
        }
    }

    /**
     * @brief Returns the user specified file path
     * 
     * @return std::string 
     */
    std::string FilePickerPopup::get_file_path()
    {
        return filePathLineEdit->text().toStdString();
    }

    /**
     * @brief Function that is called when clicking the right click menu entry. This function creates the popup hand handles accept event and calls the simulation init function
     * 
     */
    void NetlistSimulatorStudyPlugin::open_popup()
    {
        FilePickerPopup dialog(qApp->activeWindow());
        if (dialog.exec() == QDialog::Accepted)
        {
            init_simulation(dialog.get_file_path());
        }
    }
}    // namespace hal