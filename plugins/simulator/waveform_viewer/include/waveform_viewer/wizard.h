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
#include <QTableView>
#include "hal_core/netlist/gate.h"
#include "waveform_viewer.h"

namespace hal {

    class Wizard : public QWizard {
        Q_OBJECT

    public:
        Wizard(WaveformViewer *parent);

    private:
        QWizardPage *createIntroPage();
        QWizardPage *createPage1();
        QWizardPage *createPage2();
        QWizardPage *createPage3();
        QWizardPage *createPage4();
        QWizardPage *createConclusionPage();

        WaveformViewer *m_parent;
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
        Page2(QWidget *parent = 0);

    private:
        QLabel *label;
        QLineEdit *lineEdit;
    };

    class Page3 : public QWizardPage {
        Q_OBJECT

    public:
        Page3(QWidget *parent = 0);

    private:
        QLabel *label;
        QLineEdit *lineEdit;
    };

    class Page4 : public QWizardPage {
        Q_OBJECT

    public:
        Page4(QWidget *parent = 0);

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
