#include "netlist_modifier/ini_settings_popup.h"
#include "netlist_modifier/netlist_modifier.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QFileDialog>
#include <QVBoxLayout>

namespace hal
{
    int IniSettingsPopup::last_probe_limit = -1;
    int IniSettingsPopup::last_probe_type = -1;

    IniSettingsPopup::IniSettingsPopup(QWidget* parent) : QDialog(parent)
    {
        // Create radio buttons
        t_probe_option    = new QRadioButton("Use t-probe probing mode", this);
        scan_chain_option = new QRadioButton("Use scan chain probing mode", this);

        if(last_probe_type == 0){
            scan_chain_option->setChecked(true);
        }else if(last_probe_type == 1){
            t_probe_option->setChecked(true);
        }else{
            scan_chain_option->setChecked(true);
        }

        // Group the radio buttons so that only one can be selected at a time
        QButtonGroup* probe_methods = new QButtonGroup(this);
        probe_methods->addButton(t_probe_option);
        probe_methods->addButton(scan_chain_option);

        // Create the number field input
        probe_limit = new QSpinBox(this);
        probe_limit->setRange(0, 20);

        if(last_probe_limit >= 0 && last_probe_limit <= 20){
            probe_limit->setValue(last_probe_limit);
        }else{
            probe_limit->setValue(5);
        }

        // Create the OK and Cancel buttons
        okButton     = new QPushButton("OK", this);
        cancelButton = new QPushButton("Cancel", this);

        // Connect buttons
        connect(okButton, &QPushButton::clicked, this, &IniSettingsPopup::acceptSelection);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

        // Layout setup
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(t_probe_option);
        mainLayout->addWidget(scan_chain_option);
        mainLayout->addWidget(probe_limit);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        mainLayout->addLayout(buttonLayout);

        setLayout(mainLayout);

        setWindowTitle("Choose Settings");
    }

    void IniSettingsPopup::acceptSelection()
    {
        if (scan_chain_option->isChecked())
        {
            last_probe_type = 0;
        }
        else if (t_probe_option->isChecked())
        {
            last_probe_type = 1;
        }
        last_probe_limit = probe_limit->value();
        accept();
    }

    int IniSettingsPopup::get_selected_probing_model()
    {
        if (scan_chain_option->isChecked())
        {
            return 0;
        }
        else if (t_probe_option->isChecked())
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    int IniSettingsPopup::get_probe_limit()
    {
        return probe_limit->value();
    }

    void NetlistModifierPlugin::open_popup()
    {
        IniSettingsPopup dialog(qApp->activeWindow());
        if (dialog.exec() == QDialog::Accepted)
        {
            modify_in_place(dialog.get_selected_probing_model(), dialog.get_probe_limit());
        }
    }
}    // namespace hal