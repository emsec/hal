#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QSpinBox>
#include "netlist_simulator_controller/simulation_settings.h"

namespace hal {

    class SimulationSettings;

    class SimulationSettingColorButton : public QPushButton
    {
        Q_OBJECT
    public:
        QString mColorName;
        bool mBullet;
        SimulationSettingColorButton(const QString& col, bool bullet, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent* evt) override;
    };

    class SimulationSettingColorTab : public QWidget
    {
        Q_OBJECT
        SimulationSettingColorButton* mActivateColorDialog[SimulationSettings::MaxColorSetting];

    private Q_SLOTS:
        void activateColorDialog();
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
    public:
        SimulationSettingGlobalTab(SimulationSettings* settings, QWidget* parent = nullptr);
        int maxSizeLoadable() const { return mMaxSizeLoadable->value(); }
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
