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
        Wizard(SimulationSettings *settings, WaveformViewer *parent);
        int mPage4Id;
        int mPage5Id;

    private:
        QWizardPage *createIntroPage();
        QWizardPage *createPage1();
        QWizardPage *createPage2();
        QWizardPage *createPage3();
        QWizardPage *createPage4();
        QWizardPage *createPage5();
        QWizardPage *createConclusionPage();

        WaveformViewer *m_parent;
        SimulationSettings *mSettings;
    };

    class IntroPage : public QWizardPage {
        Q_OBJECT

    public:
        IntroPage(QWidget *parent = 0);

    private:
        QLabel *label;
    };

    class Page1 : public QWizardPage {
        Q_OBJECT

    public:
        Page1(WaveformViewer *parent);

        virtual bool validatePage() override;

        std::vector<Gate*> selectedGates() const;
    private:
        QPushButton* mButAll;
        QPushButton* mButSel;
        QPushButton* mButNone;
        QTableView* mTableView;
        QDialogButtonBox* mButtonBox;

        WaveformViewer *m_parent;

        void handleSelectAll();
        void handleCurrentGuiSelection();
        void handleClearSelection();
    };

    class Page2 : public QWizardPage {
        Q_OBJECT

    public:
        Page2(WaveformViewer *parent);

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

        WaveformViewer *m_parent;

        int netIndex() const { return mComboNet->currentIndex(); }
        int period() const { return mSpinPeriod->value(); }
        int startValue() const { return mSpinStartValue->value(); }
        int duration() const { return mSpinDuration->value(); }
        bool dontUseClock() const { return mDontUseClock->isChecked(); }
    };

    class Page3 : public QWizardPage {
        Q_OBJECT

    public:
        Page3(WaveformViewer *parent, Wizard *wizard);

        virtual bool validatePage() override;
        int nextId() const override;

    private:
        QVBoxLayout *mLayout;
        bool mVerilator;

        WaveformViewer *m_parent;
        Wizard *m_wizard;
    };

    class Page4 : public QWizardPage {
        Q_OBJECT

    private Q_SLOTS:
        void handleCellChanged(int irow, int icolumn);

    public:
        Page4(SimulationSettings *settings, WaveformViewer *parent);

        virtual bool validatePage() override;
    private:
        QTableWidget *mTableWidget;

        WaveformViewer *m_parent;
        SimulationSettings *mSettings;
    };

    class Page5 : public QWizardPage {
        Q_OBJECT

    public:
        Page5(QWidget *parent = 0);

    private:
        QLabel *label;
        QLineEdit *lineEdit;
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
