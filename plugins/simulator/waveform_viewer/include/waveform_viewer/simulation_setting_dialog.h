#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include "netlist_simulator_controller/simulation_settings.h"
#include "gui/main_window/color_selection.h"

namespace hal {

    class SimulationSettings;

    class SimulationSettingColorTab : public QWidget
    {
        Q_OBJECT
        ColorSelection* mActivateColorDialog[SimulationSettings::MaxColorSetting];

    public:
        SimulationSettingColorTab(SimulationSettings* settings, QWidget* parent = nullptr);
        QString colorSetting(int inx) const;
    };

    class SimulationSettingPropertiesTab : public QTableWidget
    {
        Q_OBJECT
    private Q_SLOTS:
        void handleCellChanged(int irow, int icolumn);
    public:
        SimulationSettingPropertiesTab(SimulationSettings* settings, QWidget* parent = nullptr);
        QMap<QString,QString> engineProperties() const;
    };

    class SimulationSettingGlobalTab : public QWidget
    {
        Q_OBJECT
        QSpinBox* mMaxSizeLoadable;
        QSpinBox* mMaxSizeEditor;
        QCheckBox* mCustomBaseDicectory;
        QLineEdit* mEditBaseDirectory;
    private Q_SLOTS:
        void customBaseDirectoryToggled(bool on);
    public:
        SimulationSettingGlobalTab(SimulationSettings* settings, QWidget* parent = nullptr);
        int maxSizeLoadable() const { return mMaxSizeLoadable->value(); }
        int maxSizeEditor() const { return mMaxSizeEditor->value(); }
        QString baseDirectory() const { return mEditBaseDirectory->text(); }
        bool isCustomBaseDirectory() const { return mCustomBaseDicectory->isChecked(); }
    };

    class SimulationSettingDialog : public QDialog
    {
        Q_OBJECT
        SimulationSettings* mSettings;
        QTabWidget* mTabWidget;
        QDialogButtonBox* mButtonBox;
    private Q_SLOTS:
        void accept() override;
    public:
        SimulationSettingDialog(SimulationSettings* settings, QWidget* parent = nullptr);
    };
}
