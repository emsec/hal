#pragma once

#include <QDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

namespace hal
{
    class IniSettingsPopup : public QDialog
    {
        Q_OBJECT

    public:
        explicit IniSettingsPopup(QWidget* parent = nullptr);
        int get_selected_probing_model();
        int get_probe_limit();

    private:
        QRadioButton* t_probe_option;
        QRadioButton* scan_chain_option;
        QSpinBox* probe_limit;
        QPushButton* okButton;
        QPushButton* cancelButton;

        void acceptSelection();
    };
}    // namespace hal