#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <vector>
#include <QDialogButtonBox>
#include <QTableView>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QSortFilterProxyModel>
#include <QCheckBox>
#include <QTextEdit>
#include <QFile>
#include "hal_core/netlist/gate.h"
#include "wave_widget.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "netlist_simulator_controller/simulation_process.h"
#include "waveform_viewer/wavedata_table_editor.h"

namespace hal {

    /**
     * The wizard that guides the user through setting up and running a simulation.
     */
    class Wizard : public QWizard {
        Q_OBJECT

    public:
        Wizard(SimulationSettings *settings, NetlistSimulatorController *controller, WaveWidget *parent);
        int mPageEnginePropertiesId;
        int mPageInputDataId;

    private:
        NetlistSimulatorController *mController;
        SimulationSettings *mSettings;
    };

    /**
     * The wizard page on which the simulated gates are selected.
     */
    class PageSelectGates : public QWizardPage {
        Q_OBJECT

    public:
        PageSelectGates(NetlistSimulatorController *controller, QWidget *parent=nullptr);

        virtual bool validatePage() override;

        std::vector<Gate*> selectedGates() const;

    public Q_SLOTS:
        void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    private:
        QPushButton* mButAll;
        QPushButton* mButSel;
        QPushButton* mButNone;
        QTableView* mTableView;
        QDialogButtonBox* mButtonBox;

        NetlistSimulatorController *mController;

    private Q_SLOTS:
        void handleCurrentGuiSelection();
    };

    /**
     * The wizard page on which the clock net and its period are configured.
     */
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

    /**
     * The wizard page on which the simulation engine is chosen.
     */
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

    /**
     * The wizard page on which the properties of the chosen simulation engine are set.
     */
    class PageEngineProperties : public QWizardPage {
        Q_OBJECT

    private Q_SLOTS:
        void updateComboBoxes(const QString &selectedText);
        void handleCellChanged(int irow, int icolumn);

    public:
        PageEngineProperties(SimulationSettings *settings, NetlistSimulatorController *controller, QWidget* parent=nullptr);

        virtual bool validatePage() override;
    private:
        QTableWidget *mTableWidget;

        NetlistSimulatorController *mController;
        SimulationSettings *mSettings;
        QStringList mAllItems;
    };

    /**
     * The wizard page on which the input stimuli are entered or imported.
     */
    class PageInputData : public QWizardPage {
        Q_OBJECT

    public:
        PageInputData(NetlistSimulatorController *controller, QWidget *parent=nullptr);

        virtual bool validatePage() override;
        virtual void initializePage() override;

    private Q_SLOTS:
        void handleRadioToggled(bool checked);
        void openFileBrowser();
        void handleFileImport();
        void updateSubtitle();

    private:
        NetlistSimulatorController* mController;
        QFrame* mFrameFile;
        QFrame* mFrameTable;
        QLineEdit* mEditFilename;
        QCheckBox* mDisplayHexValues;
        QPushButton* mButFiledialog;
        QPushButton* mButFileimport;
        QRadioButton* mRadFile;
        QRadioButton* mRadEditor;
        WavedataTableEditor* mTableEditor;
        bool mDisableToggleHandler;
        static bool canFileImport(const QString& filename);
    };

    /**
     * Collects the log output of the running simulation so that the wizard can display it.
     */
    class SimulationProcessOutput : public SimulationLogReceiver
    {
        Q_OBJECT
    public Q_SLOTS:
        void handleLog(const QString &txt) override;
    public:
        SimulationProcessOutput(QWidget* parent, QLayout* layout);
        void readFile(QFile& ff);
    private:
        QTextEdit* mTextEdit;
    };

    /**
     * The wizard page that runs the simulation and shows its progress.
     */
    class PageRunSimulation : public QWizardPage {
        Q_OBJECT

    public:
        PageRunSimulation(NetlistSimulatorController *controller, QWidget* parent=nullptr);
        virtual bool validatePage() override;
        virtual int nextId() const override;

    private Q_SLOTS:
        void handleStartClicked();
        void handleLogfileRead();
        void handleStateChanged(hal::NetlistSimulatorController::SimulationState state);

    public Q_SLOTS:
        void handleEngineFinished(bool success);

    private:
        NetlistSimulatorController *mController;
        SimulationProcessOutput* mProcessOutput;
        QPushButton* mStart;
        QLabel* mState;
        QFile mLogfile;
        QByteArray mLogText;
    };

    /**
     * The wizard page on which the simulation results are loaded into the waveform viewer.
     */
    class PageLoadResults : public QWizardPage {
        Q_OBJECT

    public:
        PageLoadResults(NetlistSimulatorController *controller, WaveWidget* ww, QWidget* parent=nullptr);
        virtual bool validatePage() override;
        virtual void initializePage() override;

    private Q_SLOTS:
        void useGuiSelection();

    private:
        NetlistSimulatorController *mController;
        WaveWidget* mWaveWidget;
        WaveSelectionTable* mWaveModel;
        QSortFilterProxyModel* mProxyModel;
        QPushButton* mButAll;
        QPushButton* mButGui;
        QPushButton* mButNone;
        QTableView* mTableView;
    };

}
#endif
