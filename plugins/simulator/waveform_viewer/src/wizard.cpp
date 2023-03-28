#include "waveform_viewer/wizard.h"
#include "gui/module_dialog/gate_select_model.h"

#include <QHeaderView>

#include "gui/gui_globals.h"

namespace hal {

    Wizard::Wizard(WaveformViewer *parent)
        : QWizard(parent), m_parent(parent)
    {
        setWindowTitle(tr("Empty Wizard"));

        addPage(createIntroPage());
        addPage(createPage1());
        addPage(createPage2());
        addPage(createPage3());
        addPage(createPage4());
        addPage(createConclusionPage());
    }

    QWizardPage *Wizard::createIntroPage()
    {
        QWizardPage *page = new IntroPage;
        page->setTitle(tr("Introduction"));
        page->setSubTitle(tr("Introduction about Wizard"));
        return page;
    }

    QWizardPage *Wizard::createPage1()
    {
        QWizardPage *page = new Page1(m_parent);
        page->setTitle(tr("Step 1"));
        page->setSubTitle(tr("Select Gates"));
        return page;
    }

    QWizardPage *Wizard::createPage2()
    {
        QWizardPage *page = new Page2;
        page->setTitle(tr("Step 2"));
        page->setSubTitle(tr("Clock settings"));
        return page;
    }

    QWizardPage *Wizard::createPage3()
    {
        QWizardPage *page = new Page3;
        page->setTitle(tr("Step 3"));
        page->setSubTitle(tr("Engine settings"));
        return page;
    }

    QWizardPage *Wizard::createPage4()
    {
        QWizardPage *page = new Page4;
        page->setTitle(tr("Step 4"));
        page->setSubTitle(tr("Load input Data"));
        return page;
    }

    QWizardPage *Wizard::createConclusionPage()
    {
        QWizardPage *page = new ConclusionPage;
        page->setTitle(tr("End"));
        page->setSubTitle(tr("Run Simulation"));
        return page;
    }

    IntroPage::IntroPage(QWidget *parent) : QWizardPage(parent)
    {
        label = new QLabel(tr("What is the Waveform Simulation"));
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        setLayout(layout);
    }

    Page1::Page1(WaveformViewer *parent)
      : QWizardPage(parent), m_parent(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        mButAll = new QPushButton("All gates", this);
        connect(mButAll,&QPushButton::clicked,this,&Page1::handleSelectAll);
        layout->addWidget(mButAll,0,0);
        mButSel = new QPushButton("Current GUI selection", this);
        connect(mButSel,&QPushButton::clicked,this,&Page1::handleCurrentGuiSelection);
        layout->addWidget(mButSel,0,1);
        mButNone = new QPushButton("Clear selection", this);
        connect(mButNone,&QPushButton::clicked,this,&Page1::handleClearSelection);
        layout->addWidget(mButNone,0,2);
        mTableView = new QTableView(this);

        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::MultiSelection);

        GateSelectProxy* prox = new GateSelectProxy(this);
        // connect(sbar, &Searchbar::textEdited, prox, &GateSelectProxy::searchTextChanged);

        GateSelectModel* modl = new GateSelectModel(false,QSet<u32>(),mTableView);
        prox->setSourceModel(modl);
        mTableView->setModel(prox);

        // connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &GateSelectView::handleCurrentChanged);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(2, Qt::AscendingOrder);
        mTableView->resizeColumnsToContents();
        mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->verticalHeader()->hide();
        layout->addWidget(mTableView,1,0,1,3);
        //mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        //connect(mButtonBox, &QDialogButtonBox::accepted, static_cast<QDialog*>(this), &QDialog::accept);
        //connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        //layout->addWidget(mButtonBox,2,1,1,2);

        // vllt hinzufügen dass Next Button disabled ist wenn noch nix ausgewählt ist
    }

    void Page1::handleSelectAll()
    {
        mTableView->selectAll();
    }

    void Page1::handleCurrentGuiSelection()
    {
        QSet<u32> guiGateSel = gSelectionRelay->selectedGates();

        const QAbstractItemModel* modl = mTableView->model(); // proxy model
        int nrows = modl->rowCount();
        mTableView->clearSelection();

        bool ok;

        for (int irow = 0; irow<nrows; irow++)
        {
            u32 gid = modl->data(modl->index(irow,0)).toUInt(&ok);
            if (!ok) continue;
            if (guiGateSel.contains(gid))
                mTableView->selectRow(irow);
        }
    }

    void Page1::handleClearSelection()
    {
        mTableView->clearSelection();
    }

    std::vector<hal::Gate*> Page1::selectedGates() const
    {
        std::vector<Gate*> retval;
        QItemSelectionModel *sm = mTableView->selectionModel();
        if (!sm->hasSelection()) return retval;
        QSet<u32> selGates;
        bool ok;
        for (const QModelIndex& inx : sm->selectedRows(0))
        {
            u32 gid = mTableView->model()->data(inx).toUInt(&ok);
            if (!ok) continue;
            selGates.insert(gid);
        }

        for (u32 gid: selGates)
        {
            Gate* g = gNetlist->get_gate_by_id(gid);
            if (g) retval.push_back(g);
        }
        return retval;
    }

    bool Page1::validatePage()
    {
        m_parent->setGates(selectedGates());


        return true;
        // Benutzerdefinierte Validierung durchführen
        //bool userInputIsValid = validateUserInput();

        //if (!userInputIsValid)
        //{
            // Wenn die Validierung fehlschlägt, Fehlermeldung anzeigen
         //   QMessageBox::warning(this, tr("Fehler"), tr("Ungültige Benutzereingabe."));
        //}

        // Rückgabe des Ergebnisses der Validierung
        //return userInputIsValid;
    }

    Page2::Page2(QWidget *parent): QWizardPage(parent)
    {
        label = new QLabel(tr("Clock:"));
        lineEdit = new QLineEdit;
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->addWidget(lineEdit);
        setLayout(layout);
    }

    Page3::Page3(QWidget *parent): QWizardPage(parent)
    {
        label = new QLabel(tr("Engine:"));
        lineEdit = new QLineEdit;
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->addWidget(lineEdit);
        setLayout(layout);
    }

    Page4::Page4(QWidget *parent): QWizardPage(parent)
    {
        label = new QLabel(tr("Input Data:"));
        lineEdit = new QLineEdit;
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->addWidget(lineEdit);
        setLayout(layout);
    }

    ConclusionPage::ConclusionPage(QWidget *parent): QWizardPage(parent)
    {
        label = new QLabel(tr("Run Simulation"));
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        setLayout(layout);
    }

}
