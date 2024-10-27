#pragma once

#include <QDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

/**
 * @brief Header file for the pop up used to select the options for the ini file like probing type and probe limit
 * 
 */

namespace hal
{
    class IniSettingsPopup : public QDialog
    {
        Q_OBJECT

    public:
        explicit IniSettingsPopup(QWidget* parent = nullptr);
        int get_selected_probing_model();
        int get_probe_limit();

        static int last_probe_limit;
        static int last_probe_type;

    private:
        QRadioButton* t_probe_option;
        QRadioButton* scan_chain_option;
        QSpinBox* probe_limit;
        QPushButton* okButton;
        QPushButton* cancelButton;

        void acceptSelection();
    };
}    // namespace hal