#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <vector>
#include <map>
#include <QDialogButtonBox>
#include <QTableView>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include "hal_core/netlist/gate.h"
#include "waveform_viewer.h"
#include "netlist_simulator_controller/simulation_settings.h"

namespace hal {

    class Wizard : public QWizard {
        Q_OBJECT

    public:
        Wizard(SimulationSettings *settings, NetlistSimulatorController *controller, WaveformViewer *parent);
        int mPageEnginePropertiesId;
        int mPageInputDataId;

    private:
        NetlistSimulatorController *mController;
        SimulationSettings *mSettings;
    };

    class PageSelectGates : public QWizardPage {
        Q_OBJECT

    public:
        PageSelectGates(NetlistSimulatorController *controller, QWidget *parent=nullptr);

        virtual bool validatePage() override;

        std::vector<Gate*> selectedGates() const;
    private:
        QPushButton* mButAll;
        QPushButton* mButSel;
        QPushButton* mButNone;
        QTableView* mTableView;
        QDialogButtonBox* mButtonBox;

        NetlistSimulatorController *mController;

        void handleSelectAll();
        void handleCurrentGuiSelection();
        void handleClearSelection();
    };

    class PageClock : public QWizardPage {
        Q_OBJECT

    public:
        PageClock(NetlistSimulatorController *controller, QWidget *parent=nullptr);

        virtual bool validatePage() override;
        virtual void initializePage() override;

    private Q_SLOTS:
        void dontUseClockChanged(bool state);

    private:
        QComboBox* mComboNet;
        QSpinBox* mSpinPeriod;
        QSpinBox* mSpinStartValue;
        QSpinBox* mSpinDuration;
        QCheckBox* mDontUseClock;

        NetlistSimulatorController *mController;

        int netIndex() const { return mComboNet->currentIndex(); }
        int period() const { return mSpinPeriod->value(); }
        int startValue() const { return mSpinStartValue->value(); }
        int duration() const { return mSpinDuration->value(); }
        bool dontUseClock() const { return mDontUseClock->isChecked(); }
    };

    class PageEngine : public QWizardPage {
        Q_OBJECT

    public:
        PageEngine(NetlistSimulatorController *controller, Wizard* parent=nullptr);

        virtual bool validatePage() override;
        int nextId() const override;

    private:
        QVBoxLayout *mLayout;
        bool mVerilator;

        NetlistSimulatorController *mController;
        Wizard* m_wizard;
    };

    class PageEngineProperties : public QWizardPage {
        Q_OBJECT

    private Q_SLOTS:
        void handleCellChanged(int irow, int icolumn);

    public:
        PageEngineProperties(SimulationSettings *settings, NetlistSimulatorController *controller, QWidget* parent=nullptr);

        virtual bool validatePage() override;
    private:
        QTableWidget *mTableWidget;

        NetlistSimulatorController *mController;
        SimulationSettings *mSettings;
    };

    class PageInputData : public QWizardPage {
        Q_OBJECT

    public:
        PageInputData(NetlistSimulatorController *controller, QWidget *parent=nullptr);
    private Q_SLOTS:
        void openFileBrowser();

    private:
        NetlistSimulatorController *mController;
        QLineEdit *mEditFilename;
    };

    class ConclusionPage : public QWizardPage {
        Q_OBJECT

    public:
        ConclusionPage(QWidget *parent = 0);

    private:
        QLabel *label;
    };

}
#endif
