#include "waveform_viewer/wizard.h"
#include "waveform_viewer/wave_widget.h"
#include "gui/module_dialog/gate_select_model.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include <QHeaderView>
#include <QFileDialog>

#include "gui/gui_globals.h"

namespace hal {

    Wizard::Wizard(SimulationSettings *settings, NetlistSimulatorController *controller, WaveformViewer *parent)
        : QWizard(parent),  mController(controller), mSettings(settings)
    {
        setWindowTitle(tr("Empty Wizard"));

        addPage(new PageSelectGates(mController, this));
        addPage(new PageClock(mController, this));
        addPage(new PageEngine(mController, this));
        mPageEnginePropertiesId = addPage(new PageEngineProperties(mSettings,mController,this));
        mPageInputDataId = addPage(new PageInputData(mController,this));
        addPage(new ConclusionPage(this));
    }

    PageSelectGates::PageSelectGates(NetlistSimulatorController *controller, QWidget *parent)
      : QWizardPage(parent), mController(controller)
    {
        setTitle(tr("Step 1"));
        setSubTitle(tr("Select Gates"));

        QGridLayout* layout = new QGridLayout(this);
        mButAll = new QPushButton("All gates", this);
        connect(mButAll,&QPushButton::clicked,this,&PageSelectGates::handleSelectAll);
        layout->addWidget(mButAll,0,0);
        mButSel = new QPushButton("Current GUI selection", this);
        connect(mButSel,&QPushButton::clicked,this,&PageSelectGates::handleCurrentGuiSelection);
        layout->addWidget(mButSel,0,1);
        mButNone = new QPushButton("Clear selection", this);
        connect(mButNone,&QPushButton::clicked,this,&PageSelectGates::handleClearSelection);
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

    void PageSelectGates::handleSelectAll()
    {
        mTableView->selectAll();
    }

    void PageSelectGates::handleCurrentGuiSelection()
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

    void PageSelectGates::handleClearSelection()
    {
        mTableView->clearSelection();
    }

    std::vector<hal::Gate*> PageSelectGates::selectedGates() const
    {
        std::vector<Gate*> retval;
        QItemSelectionModel *sm = mTableView->selectionModel();
        if (!sm->hasSelection()) return retval;
        QSet<u32> selGates;
        bool ok;
        for (const QModelIndex& inx : sm->selectedRows(0) )
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

    bool PageSelectGates::validatePage()
    {
        //m_parent->setGates(selectedGates());
        //if (!mController || mController->get_state() != NetlistSimulatorController::NoGatesSelected) return false;
        mController->reset();
        mController->add_gates(selectedGates());
        for (const Net* inpNet : mController->get_input_nets())
            mController->get_waveform_by_net(inpNet);
        if (mController->get_gates().empty())
            return false;


        // wurde gate ausgewählt???
        return true;
    }


    PageClock::PageClock(NetlistSimulatorController *controller, QWidget* parent)
        : QWizardPage(parent), mController(controller)
    {
        setTitle(tr("Step 2"));
        setSubTitle(tr("Clock settings"));

        // was wenn inputs leer?

        QGridLayout* layout = new QGridLayout(this);
        mComboNet = new QComboBox(this);

        layout->addWidget(new QLabel("Select clock net:",this),0,0);
        layout->addWidget(mComboNet,0,1);

        layout->addWidget(new QLabel("Clock period:",this),1,0);
        mSpinPeriod = new QSpinBox(this);
        mSpinPeriod->setMinimum(0);
        mSpinPeriod->setMaximum(1000000);
        mSpinPeriod->setValue(10);
        layout->addWidget(mSpinPeriod,1,1);

        layout->addWidget(new QLabel("Start value:",this),2,0);
        mSpinStartValue = new QSpinBox(this);
        mSpinStartValue->setMinimum(0);
        mSpinStartValue->setMaximum(1);
        layout->addWidget(mSpinStartValue,2,1);

        layout->addWidget(new QLabel("Duration:",this),3,0);
        mSpinDuration = new QSpinBox(this);
        mSpinDuration->setMinimum(0);
        mSpinDuration->setMaximum(1000000);
        mSpinDuration->setValue(2000);
        layout->addWidget(mSpinDuration,3,1);

        mDontUseClock = new QCheckBox("Do not use clock in simulation",this);
        mDontUseClock->setCheckState(Qt::Unchecked);
        connect(mDontUseClock,&QCheckBox::stateChanged,this,&PageClock::dontUseClockChanged);
        layout->addWidget(mDontUseClock,4,0,1,2);

        //mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        //connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        //connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        //layout->addWidget(mButtonBox,5,1);
    }

    void PageClock::initializePage()
    {
        int j = 0;
        int iclk = -1;

        for (const Net* n : mController->get_input_nets())
        {
            QString netName = QString::fromStdString(n->get_name());
            QString upcase = netName.toUpper();
            if (upcase == "CLK" || upcase == "CLOCK")
                iclk = j;
            else if ((upcase.contains("CLK") || upcase.contains("CLOCK")) && j<0 )
                iclk = j;
            mComboNet->insertItem(j++,QString("%1[%2]").arg(netName).arg(n->get_id()));
        }
        if (iclk >= 0) mComboNet->setCurrentIndex(iclk);
    }

    void PageClock::dontUseClockChanged(bool state)
    {
        mComboNet->setDisabled(state);
        mSpinPeriod->setDisabled(state);
        mSpinStartValue->setDisabled(state);
        mSpinDuration->setDisabled(state);
    }

    bool PageClock::validatePage()
    {

        if (mDontUseClock->isChecked())
        {
            mController->set_no_clock_used();
        }
        else
        {
            int period = mSpinPeriod->value();
            if (period <= 0) return false;

            std::string clkNetName = mComboNet->currentText().toStdString();

            for (const Net* n : mController->get_input_nets())
            {
                if (clkNetName == n->get_name())
                {
                    const Net* clk = n;
                    mController->add_clock_period(
                        clk, period, mSpinStartValue->value()==0, mSpinDuration->value());
                    qDebug() << "clock net selected" << clk->get_name().c_str();
                    break;
                }
            }
        }

        // wurde clock ausgewählt???
        return true;
    }


    PageEngine::PageEngine(NetlistSimulatorController *controller, Wizard *parent)
        : QWizardPage(parent), mController(controller), m_wizard(parent)
    {
        setTitle(tr("Step 3"));
        setSubTitle(tr("Engine settings"));

        mLayout = new QVBoxLayout(this);

        for (SimulationEngineFactory* sef : *SimulationEngineFactories::instance())
        {
            QRadioButton *radioButton = new QRadioButton(QString::fromStdString(sef->name()), this);
            mLayout->addWidget(radioButton);

            if (QString::fromStdString(sef->name()) == "verilator")
            {
                radioButton->setChecked(true);
            }
        }

        setLayout(mLayout);
    }

    bool PageEngine::validatePage()
    {

        QString selectedEngineName;

        for (int i = 0; i < mLayout->count(); ++i)
        {
            QRadioButton *radioButton = qobject_cast<QRadioButton *>(mLayout->itemAt(i)->widget());
            if (radioButton && radioButton->isChecked())
            {
                selectedEngineName = radioButton->text();
                break;
            }
        }
        if (selectedEngineName.toStdString() == "verilator")
        {
            mVerilator = true;
        }
        else
        {
            mVerilator = false;
        }


        mController->create_simulation_engine(selectedEngineName.toStdString());
        std::cout << selectedEngineName.toStdString() << std::endl;


        // wurde clock ausgewählt???
        return true;
    }

    int PageEngine::nextId() const
    {
        if (!m_wizard) return QWizardPage::nextId();
        if (mVerilator)
        {
            return m_wizard->mPageEnginePropertiesId;
        }
        else
        {
            return m_wizard->mPageInputDataId;
        }
    }



    PageEngineProperties::PageEngineProperties(SimulationSettings *settings, NetlistSimulatorController *controller, QWidget *parent)
        : QWizardPage(parent), mController(controller), mSettings(settings)
    {
        setTitle(tr("Step 4"));
        setSubTitle(tr("Engine properties"));

        mTableWidget = new QTableWidget(this);

        QMap<QString,QString> engProp = settings->engineProperties();
        mTableWidget->setColumnCount(2);
        mTableWidget->setColumnWidth(0,250);
        mTableWidget->setColumnWidth(1,350);
        mTableWidget->setRowCount(engProp.size()+3);
        mTableWidget->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");

        int irow = 0;
        for (auto it = engProp.constBegin(); it != engProp.constEnd(); ++it)
        {
            QTableWidgetItem wi(it.key());
            mTableWidget->setItem(irow, 0, new QTableWidgetItem(it.key()));
            mTableWidget->setItem(irow, 1, new QTableWidgetItem(it.value()));
            ++irow;
        }
        mTableWidget->horizontalHeader()->setStretchLastSection(true);

        mTableWidget->horizontalHeader()->setStretchLastSection(true);
        connect(mTableWidget, &QTableWidget::cellChanged, this, &PageEngineProperties::handleCellChanged);


        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(mTableWidget);
        setLayout(layout);

    }

    void PageEngineProperties::handleCellChanged(int irow, int icolumn)
    {
        if ((icolumn == 1 && irow >= mTableWidget->rowCount()-2) ||
            (icolumn == 0 && irow >= mTableWidget->rowCount()-1))
            mTableWidget->setRowCount(mTableWidget->rowCount()+1);
    }

    bool PageEngineProperties::validatePage()
    {

        QMap<QString,QString> engProp;
        for (int irow=0; irow < mTableWidget->rowCount(); ++irow)
        {
            const QTableWidgetItem* wi = mTableWidget->item(irow,0);
            if (!wi) continue;
            QString key = wi->text().trimmed();
            if (key.isEmpty()) continue;
            wi = mTableWidget->item(irow,1);
            QString value = wi ? wi->text().trimmed() : QString();
            engProp[key] = value;
        }
        mSettings->setEngineProperties(engProp);
        mSettings->sync();

        return true;
    }

    void PageInputData::openFileBrowser()
    {
        QString filter = QString("Saved data (%1)").arg(NetlistSimulatorController::sPersistFile);
        filter += ";; VCD files (*.vcd);; CSV files (*.csv)";

        QString filename =
                QFileDialog::getOpenFileName(this, "Load input wave file", ".", filter);

        if (filename.isEmpty()) return;

        mEditFilename->setText(filename);

        if (filename.endsWith(NetlistSimulatorController::sPersistFile))
        {
            NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
            if (ctrlPlug)
            {
                std::unique_ptr<NetlistSimulatorController> ctrlRef = ctrlPlug->restore_simulator_controller(gNetlist,filename.toStdString());
                //takeControllerOwnership(ctrlRef, true);
            }
        }
        else if (mController->can_import_data() && filename.toLower().endsWith(".vcd"))
            mController->import_vcd(filename.toStdString(),NetlistSimulatorController::GlobalInputs);
        else if (mController->can_import_data() && filename.toLower().endsWith(".csv"))
            mController->import_csv(filename.toStdString(),NetlistSimulatorController::GlobalInputs);
        else
            log_warning(mController->get_name(), "Cannot parse file '{}' (unknown extension ore wrong state).", filename.toStdString());

    }

    PageInputData::PageInputData(NetlistSimulatorController *controller, QWidget* parent)
        : QWizardPage(parent), mController(controller)
    {
        setTitle(tr("Step 5"));
        setSubTitle(tr("Load input Data"));
        QGridLayout* layout = new QGridLayout(this);

        mEditFilename = new QLineEdit(this);
        layout->addWidget(mEditFilename,0,0);
        QPushButton* but = new QPushButton("v",this);
        connect(but,&QPushButton::clicked,this,&PageInputData::openFileBrowser);
        layout->addWidget(but,0,1);
    }

    ConclusionPage::ConclusionPage(QWidget *parent): QWizardPage(parent)
    {
        setTitle(tr("End"));
        setSubTitle(tr("Run Simulation"));

        label = new QLabel(tr("Run Simulation"));
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        setLayout(layout);
    }

}
