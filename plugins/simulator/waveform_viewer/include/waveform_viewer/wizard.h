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
        QWizardPage *createIntroPage();
        QWizardPage *createPageSelectGates();
        QWizardPage *createPageClock();
        QWizardPage *createPageEngine();
        QWizardPage *createPageEngineProperties();
        QWizardPage *createPageInputData();
        QWizardPage *createConclusionPage();

        NetlistSimulatorController *mController;
        SimulationSettings *mSettings;
    };

    class IntroPage : public QWizardPage {
        Q_OBJECT

    public:
        IntroPage(QWidget *parent = 0);

    private:
        QLabel *label;
    };

    class PageSelectGates : public QWizardPage {
        Q_OBJECT

    public:
        PageSelectGates(Wizard *parent, NetlistSimulatorController *controller);

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
        PageClock(Wizard *parent, NetlistSimulatorController *controller);

        virtual bool validatePage() override;

    private Q_SLOTS:
        void dontUseClockChanged(bool state);

    private:
        QComboBox* mComboNet;
        QSpinBox* mSpinPeriod;
        QSpinBox* mSpinStartValue;
        QSpinBox* mSpinDuration;
        QCheckBox* mDontUseClock;
        QList<const Net*> mInputs;

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
        PageEngine(Wizard *parent, NetlistSimulatorController *controller);

        virtual bool validatePage() override;
        int nextId() const override;

    private:
        QVBoxLayout *mLayout;
        bool mVerilator;

        NetlistSimulatorController *mController;
        Wizard *m_wizard;
    };

    class PageEngineProperties : public QWizardPage {
        Q_OBJECT

    private Q_SLOTS:
        void handleCellChanged(int irow, int icolumn);

    public:
        PageEngineProperties(Wizard *parent, SimulationSettings *settings, NetlistSimulatorController *controller);

        virtual bool validatePage() override;
    private:
        QTableWidget *mTableWidget;

        NetlistSimulatorController *mController;
        SimulationSettings *mSettings;
    };

    class PageInputData : public QWizardPage {
        Q_OBJECT

    public:
        PageInputData(Wizard *parent, NetlistSimulatorController *controller);

    private:
        NetlistSimulatorController *mController;
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
