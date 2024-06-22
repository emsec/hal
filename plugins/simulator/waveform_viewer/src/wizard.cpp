#include "waveform_viewer/wizard.h"
#include "waveform_viewer/wave_widget.h"
#include "netlist_simulator_controller/simulation_process.h"
#include "gui/module_dialog/gate_select_model.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_process.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"


namespace hal {

    Wizard::Wizard(SimulationSettings *settings, NetlistSimulatorController *controller, WaveWidget *parent)
        : QWizard(parent),  mController(controller), mSettings(settings)
    {
        setWindowTitle(tr("Empty Wizard"));

        addPage(new PageSelectGates(mController, this));
        addPage(new PageClock(mController, this));
        addPage(new PageEngine(mController, this));
        mPageEnginePropertiesId = addPage(new PageEngineProperties(mSettings,mController,this));
        mPageInputDataId = addPage(new PageInputData(mController,this));
        addPage(new PageRunSimulation(mController,this));
        addPage(new PageLoadResults(mController,parent,this));
    }

    PageSelectGates::PageSelectGates(NetlistSimulatorController *controller, QWidget *parent)
      : QWizardPage(parent), mController(controller)
    {
        setTitle(tr("Step 1 : Select Gates"));
        setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_select_gates","PNG").scaled(128,128));

        QGridLayout* layout = new QGridLayout(this);
        mButAll = new QPushButton("All gates", this);
        layout->addWidget(mButAll,0,0);
        mButSel = new QPushButton("Current GUI selection", this);
        connect(mButSel,&QPushButton::clicked,this,&PageSelectGates::handleCurrentGuiSelection);
        layout->addWidget(mButSel,0,1);
        mButNone = new QPushButton("Clear selection", this);
        layout->addWidget(mButNone,0,2);
        mTableView = new QTableView(this);

        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::MultiSelection);

        GateSelectProxy* prox = new GateSelectProxy(this);

        GateSelectModel* modl = new GateSelectModel(false,QSet<u32>(),mTableView);
        prox->setSourceModel(modl);
        mTableView->setModel(prox);
        connect(mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PageSelectGates::onSelectionChanged);

        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(2, Qt::AscendingOrder);
        mTableView->resizeColumnsToContents();
        mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->verticalHeader()->hide();
        layout->addWidget(mTableView,1,0,1,3);
        connect(mButAll,&QPushButton::clicked,mTableView,&QTableView::selectAll);
        connect(mButNone,&QPushButton::clicked,mTableView,&QTableView::clearSelection);
        onSelectionChanged(QItemSelection(),QItemSelection());
    }

    void PageSelectGates::handleCurrentGuiSelection()
    {
        QSet<u32> guiGateSel = gSelectionRelay->selectedGates();
        for (u32 modId : gSelectionRelay->selectedModules())
        {
            Module* m = gNetlist->get_module_by_id(modId);
            if (!m) continue;
            for (Gate* g : m->get_gates(nullptr,true))
            {
                guiGateSel.insert(g->get_id());
            }
        }

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

    void PageSelectGates::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(selected);
        Q_UNUSED(deselected);
        int numSelectedGates = mTableView->selectionModel()->selectedRows().count();
        QString txt("\nSelect the gates to be used for simulation.\n");
        txt += QString::number(numSelectedGates) + " gates selected.";
        setSubTitle(txt);
    }

    bool PageSelectGates::validatePage()
    {
        mController->reset();
        mController->add_gates(selectedGates());
        for (const Net* inpNet : mController->get_input_nets())
            mController->get_waveform_by_net(inpNet);
        if (mController->get_gates().empty() || mController->get_input_nets().empty())
            return false;

        mController->make_waveform_groups();
        return true;
    }


    PageClock::PageClock(NetlistSimulatorController *controller, QWidget* parent)
        : QWizardPage(parent), mController(controller)
    {
        setTitle(tr("Step 2 : Clock settings"));
        setSubTitle(tr("\nSelect and generate the clock input\nor indicate that no clock generator is used\n(e.g. when clock signal provided in input data)"));
        setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_select_clock","PNG").scaled(128,128));

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

        mDontUseClock = new QCheckBox("Do not use clock generator in simulation",this);
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
            if (period <= 0)
            {
                return false;
                QMessageBox::warning(this, "Error", "Invalid period specified.");
            }


            std::string clkNetName = mComboNet->currentText().toStdString();

            for (const Net* n : mController->get_input_nets())
                {
                size_t pos = clkNetName.find("[");
                if (pos != std::string::npos)
                    clkNetName = clkNetName.substr(0, pos);

                if (clkNetName == n->get_name())
                {
                    const Net* clk = n;
                    mController->add_clock_period(
                        clk, period, mSpinStartValue->value()==0, mSpinDuration->value());
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
        setTitle(tr("Step 3 : Engine settings"));
        setSubTitle(tr("\nSelect the engine for the simulation\nIf the engine you are looking for is not listed,\nyou might have to load the plugin first"));
        setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_select_engine","PNG").scaled(128,128));

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
        setTitle(tr("Step 3.1 : Engine properties"));
        setSubTitle(tr("\nEnter engine properties for the verilator"));
        setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_engine_par","PNG").scaled(128,128));

        mTableWidget = new QTableWidget(this);

        QMap<QString,QString> engProp = settings->engineProperties();
        mTableWidget->setColumnCount(2);
        mTableWidget->setColumnWidth(0,250);
        mTableWidget->setColumnWidth(1,350);
        mTableWidget->setRowCount(engProp.size()+3);
        mTableWidget->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");

        mAllItems << "" << "provided_models" << "num_of_threads" << "compiler" << "ssh_server";

        for (int irow = 0; irow < mTableWidget->rowCount(); ++irow)
        {
            QComboBox *comboBox = new QComboBox(this);
            comboBox->addItems(mAllItems);
            comboBox->setEditable(true);
            mTableWidget->setCellWidget(irow, 0, comboBox);
            connect(comboBox, &QComboBox::currentTextChanged, this, &PageEngineProperties::updateComboBoxes);
        }

        int irow = 0;
        for (auto it = engProp.constBegin(); it != engProp.constEnd(); ++it)
        {
            QComboBox *comboBox = qobject_cast<QComboBox *>(mTableWidget->cellWidget(irow, 0));
            if (comboBox)
            {
                int index = comboBox->findText(it.key());
                if (index != -1)
                {
                     comboBox->setCurrentIndex(index);
                }
                else
                {
                    comboBox->addItem(it.key());
                    comboBox->setCurrentText(it.key());
                }
             }
             mTableWidget->setItem(irow, 1, new QTableWidgetItem(it.value()));
             ++irow;
         }
         mTableWidget->horizontalHeader()->setStretchLastSection(true);

         mTableWidget->horizontalHeader()->setStretchLastSection(true);
         connect(mTableWidget, &QTableWidget::cellChanged, this, &PageEngineProperties::handleCellChanged);

         QVBoxLayout *layout = new QVBoxLayout;
         layout->addWidget(mTableWidget);
         setLayout(layout);

        /// verilator will need verilog netlist writer plugin, go and get it unless already loaded
        if (gPluginRelay->mGuiPluginTable)
            gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacNetlistWriter,".v");
    }

    void PageEngineProperties::updateComboBoxes(const QString &selectedText)
    {
        if (selectedText == "")
            return;
        QStringList selectedItems;
        for (int irow = 0; irow < mTableWidget->rowCount(); ++irow)
        {
            QComboBox *comboBox = qobject_cast<QComboBox *>(mTableWidget->cellWidget(irow, 0));
            if (comboBox)
            {
                selectedItems << comboBox->currentText();
            }
        }

        for (int irow = 0; irow < mTableWidget->rowCount(); ++irow)
        {
            QComboBox *comboBox = qobject_cast<QComboBox *>(mTableWidget->cellWidget(irow, 0));
            if (comboBox)
            {

                comboBox->blockSignals(true);
                QString currentText = comboBox->currentText();
                comboBox->clear();

                for (const QString &item : mAllItems)
                {
                    if (!selectedItems.contains(item) || item == currentText || item == "")
                    {
                        comboBox->addItem(item);
                    }
                }
                comboBox->setCurrentText(currentText);

                comboBox->blockSignals(false);
            }
        }
    }

    void PageEngineProperties::handleCellChanged(int irow, int icolumn)
    {
        if ((icolumn == 1 && irow >= mTableWidget->rowCount()-2) ||
            (icolumn == 0 && irow >= mTableWidget->rowCount()-1))
            mTableWidget->setRowCount(mTableWidget->rowCount()+1);
    }

    bool PageEngineProperties::validatePage()
    {

        QMap<QString, QString> engProp;
        for (int irow = 0; irow < mTableWidget->rowCount(); ++irow)
        {
            QComboBox *comboBox = qobject_cast<QComboBox *>(mTableWidget->cellWidget(irow, 0));
            if (!comboBox) continue;
            QString key = comboBox->currentText().trimmed();
            if (key.isEmpty() || key == "") continue;

            const QTableWidgetItem *wi = mTableWidget->item(irow, 1);
            QString value = wi ? wi->text().trimmed() : QString();
            if (value.isEmpty()) continue;

            // weißt man dem selben key zwei verschiedene values zu wird die erste überschrieben.
            engProp[key] = value;
        }
        mSettings->setEngineProperties(engProp);
        mSettings->sync();

        return true;
    }


    PageInputData::PageInputData(NetlistSimulatorController *controller, QWidget* parent)
        : QWizardPage(parent), mController(controller), mDisableToggleHandler(false)
    {
        setTitle(tr("Step 4 : Simulation Input Data"));
        setSubTitle(tr("\nNo input data file selected so far"));
        setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_select_input","PNG").scaled(128,128));


        QVBoxLayout* layout = new QVBoxLayout(this);
        QHBoxLayout* hlay = new QHBoxLayout;

        mRadFile = new QRadioButton("Simulation input from file", this);
        connect(mRadFile, &QRadioButton::toggled, this, &PageInputData::handleRadioToggled);
        layout->addWidget(mRadFile);

        mEditFilename = new QLineEdit(this);
        connect(mEditFilename, &QLineEdit::textChanged, this, &PageInputData::updateSubtitle);

        hlay->addWidget(mEditFilename);

        mButFiledialog = new QPushButton(this);
        QSize iconSize(17,17);
        QIcon iconFdAct = gui_utility::getStyledSvgIcon("all->#3192C5", ":/icons/folder");
        QIcon iconFdDis = gui_utility::getStyledSvgIcon("all->#515050", ":/icons/folder");
        QIcon iconFd;
        iconFd.addPixmap(iconFdAct.pixmap(iconSize),QIcon::Normal);
        iconFd.addPixmap(iconFdDis.pixmap(iconSize),QIcon::Disabled);
        mButFiledialog->setIcon(iconFd);
        mButFiledialog->setIconSize(QSize(17, 17));
        connect(mButFiledialog, &QPushButton::clicked, this, &PageInputData::openFileBrowser);

        hlay->addWidget(mButFiledialog);
        layout->addLayout(hlay);
        layout->addSpacing(0);
        mRadEditor = new QRadioButton("Enter simulation input manually", this);
        connect(mRadEditor, &QRadioButton::toggled, this, &PageInputData::handleRadioToggled);
        layout->addWidget(mRadEditor);
        mTableEditor = new WavedataTableEditor(this);
        connect(mTableEditor, &WavedataTableEditor::lineAdded, this, &PageInputData::updateSubtitle);
        layout->addWidget(mTableEditor);
        handleRadioToggled(true);
    }

    void PageInputData::initializePage()
    {
        // enter clock data must be omitted if clock is autogenerated
        mTableEditor->setup(mController->get_input_column_headers(), !mController->is_no_clock_used());
        handleRadioToggled(true);
    }

    void PageInputData::handleRadioToggled(bool checked)
    {
        if (mDisableToggleHandler) return;
        bool setManualInput = false;
        if (sender() == mRadEditor)
            setManualInput = checked;
        else
            setManualInput = !checked;
        mDisableToggleHandler = true;
        if (setManualInput)
        {
            mRadEditor->setChecked(true);
            mRadFile->setChecked(false);
            mEditFilename->setEnabled(false);
            mButFiledialog->setEnabled(false);
            mTableEditor->setEnabled(true);
        }
        else
        {
            mRadEditor->setChecked(false);
            mRadFile->setChecked(true);
            mEditFilename->setEnabled(true);
            mButFiledialog->setEnabled(true);
            mTableEditor->setEnabled(false);
        }
        mDisableToggleHandler = false;
        updateSubtitle();
    }

    void PageInputData::openFileBrowser()
    {
        QString filter = QString("Saved data (%1)").arg(NetlistSimulatorController::sPersistFile);
        filter += ";; VCD files (*.vcd);; CSV files (*.csv)";

        QString filename =
                QFileDialog::getOpenFileName(this, "Load input wave file", ".", filter);

        if (filename.isEmpty()) return;

        mEditFilename->setText(filename);
        updateSubtitle();
    }

    void PageInputData::updateSubtitle()
    {
        QString fileName = mEditFilename->text();
        QString subtitle;

        if (mRadFile->isChecked())
        {
            if (fileName.isEmpty())
            {
                subtitle = "No input data file selected so far";
            }
            else
            {
                QFileInfo fileInfo(fileName);

                if (!fileInfo.isFile() || !fileInfo.isReadable())
                {
                    subtitle = "File '" + fileName + "' is not readable";
                }
                else if (!fileName.toLower().endsWith(".vcd") && !fileName.toLower().endsWith(".csv"))
                {
                    subtitle = "Parsing input files with extension '." + fileInfo.suffix() + "' is not supported";
                }
                else
                {
                    subtitle = "Run simulation with data file '" + fileName + "'";
                }
            }
        }
        else
        {
            int nLines = mTableEditor->validLines();
            if (nLines < 2)
                subtitle = "Please enter input data in table below";
            else
                subtitle = QString("%1 input events entered in table so far").arg(nLines);
        }
        setSubTitle(subtitle);
    }

    bool PageInputData::validatePage()
    {
        if (mRadFile->isChecked())
        {
            QString fileName = mEditFilename->text();
            if (fileName.isEmpty())
            {
                QMessageBox::warning(this, "Error", "Please select a file to load.");
                return false;
            }

            QFileInfo fileInfo(fileName);
            if (!fileInfo.exists() || !fileInfo.isFile())
            {
                QMessageBox::warning(this, "Error", "Please select a valid file to load.");
                return false;
            }

            if (fileName.endsWith(NetlistSimulatorController::sPersistFile))
            {
                NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
                if (ctrlPlug)
                {
                    std::unique_ptr<NetlistSimulatorController> ctrlRef = ctrlPlug->restore_simulator_controller(gNetlist, fileName.toStdString());
                    //mController->takeControllerOwnership(ctrlRef.get(), true); // save controller
                }
            }
            else if (mController->can_import_data() && fileName.toLower().endsWith(".vcd"))
                mController->import_vcd(fileName.toStdString(), NetlistSimulatorController::GlobalInputs);
            else if (mController->can_import_data() && fileName.toLower().endsWith(".csv"))
                mController->import_csv(fileName.toStdString(), NetlistSimulatorController::GlobalInputs);
            else
            {
                QMessageBox::warning(this, "Error", "Please select a file ending with .vcd or .csv.");
                log_warning(mController->get_name(), "Cannot parse file '{}' (unknown extension or wrong state).", fileName.toStdString());
                return false;
            }
        }
        else
        {
            if (mTableEditor->validLines() < 2)
            {
                QMessageBox::warning(this, "Error", "Not enough input data entered into table");
                return false;
            }
            else
            {
                mTableEditor->generateSimulationInput(QString::fromStdString(mController->get_working_directory()));
                mController->get_waves()->updateFromSaleae();
                mController->simulate(mTableEditor->maxTime());
            }
        }

        return true;
    }

    SimulationProcessOutput::SimulationProcessOutput(QWidget* parent, QLayout *layout)
        : SimulationLogReceiver(parent)
    {
        mTextEdit = new QTextEdit(parent);
        layout->addWidget(mTextEdit);
    }

    void SimulationProcessOutput::handleLog(const QString &txt)
    {
        mTextEdit->moveCursor(QTextCursor::End);
        mTextEdit->insertHtml(txt);
        mTextEdit->moveCursor(QTextCursor::End);
    }

    void SimulationProcessOutput::readFile(QFile &ff)
    {
        mTextEdit->setHtml(QString::fromUtf8(ff.readAll()));
        mTextEdit->moveCursor(QTextCursor::End);
    }

    PageRunSimulation::PageRunSimulation(NetlistSimulatorController *controller, QWidget *parent)
        : QWizardPage(parent), mController(controller)
    {
        setTitle(tr("Step 5 : Run Simulation"));
        setSubTitle(tr("\nStart simulation based on controller settings from previous steps"));
        setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_run_simulation","PNG").scaled(128,128));

        QIcon runIconEnabled  = gui_utility::getStyledSvgIcon("all->#20FF80", ":/icons/run");
        QIcon runIconDisabled = gui_utility::getStyledSvgIcon("all->#808080", ":/icons/run");
        QIcon runIcon;
        runIcon.addPixmap(runIconEnabled.pixmap(32),QIcon::Normal);
        runIcon.addPixmap(runIconDisabled.pixmap(32),QIcon::Disabled);
        QVBoxLayout* layout = new QVBoxLayout(this);
        mProcessOutput = new SimulationProcessOutput(this,layout);
        mStart = new QPushButton("Run Simulation",this);
        mStart->setIcon(runIcon);
        connect(mStart,&QPushButton::clicked,this,&PageRunSimulation::handleStartClicked);
        layout->addWidget(mStart);
        mState = new QLabel("Ready to start simulation",this);
        layout->addWidget(mState);
        setLayout(layout);
    }

    void PageRunSimulation::handleStartClicked()
    {
        mStart->setDisabled(true);
        connect(mController,&NetlistSimulatorController::stateChanged,this,&PageRunSimulation::handleStateChanged);
        connect(mController,&NetlistSimulatorController::engineFinished,this,&PageRunSimulation::handleEngineFinished);
        mController->setLogReceiver(mProcessOutput);
        if (!mController->run_simulation())
        {
            log_info(mController->get_name(),"Wizzard failed to start simulation");
            mStart->setEnabled(true);
            handleStateChanged(mController->get_state());
        }
        log_info(mController->get_name(),"Simulation started ...");
        QString fname = QDir(QString::fromStdString(mController->get_working_directory())).absoluteFilePath(SimulationProcessLog::sLogFilename);
        mLogfile.setFileName(fname);
        if (mLogfile.open(QIODevice::ReadOnly))
        {
            connect(&mLogfile,&QIODevice::readyRead,this,&PageRunSimulation::handleLogfileRead);
        }
    }

    void PageRunSimulation::handleStateChanged(hal::NetlistSimulatorController::SimulationState state)
    {
        switch (state) {
        case NetlistSimulatorController::NoGatesSelected: mState->setText("Controller state: NoGatesSelected");            break;
        case NetlistSimulatorController::ParameterSetup:  mState->setText("Controller state: ParameterSetup");             break;
        case NetlistSimulatorController::ParameterReady:  mState->setText("Ready to start simulation");                    break;
        case NetlistSimulatorController::SimulationRun:   mState->setText("Simulation engine running, please wait ...");   break;
        case NetlistSimulatorController::ShowResults:     mState->setText("Simulation successful");                        break;
        case NetlistSimulatorController::EngineFailed:    mState->setText("Simulation engine failed");                     break;
        }
    }

    void PageRunSimulation::handleLogfileRead()
    {
        mProcessOutput->readFile(mLogfile);
    }

    void PageRunSimulation::handleEngineFinished(bool success)
    {
        if (!success) return;
        if (!mController->get_results())
            log_warning(mController->get_name(), "Cannot get simulation results");
        handleLogfileRead();
        handleStateChanged(mController->get_state());
    }

    bool PageRunSimulation::validatePage()
    {
        return (mController->get_state()==NetlistSimulatorController::ShowResults ||
                mController->get_state()==NetlistSimulatorController::EngineFailed);
    }

    int PageRunSimulation::nextId() const
    {
        if (mController->get_state()==NetlistSimulatorController::EngineFailed) return -1;
        return QWizardPage::nextId();
    }

    PageLoadResults::PageLoadResults(NetlistSimulatorController *controller, WaveWidget *ww, QWidget *parent)
        : QWizardPage(parent), mController(controller), mWaveWidget(ww)
    {
         setTitle(tr("Simulation Done : Load Simulation Results"));
         setSubTitle("\nThis page is not ready yet.\nPlease invoke load results from toolbar.");
      //   setSubTitle("\nSelect simulated waveform to be loaded into viewer. If selection from graphical netlist is preferred please exit wizard, select nets and invoke load results from toolbar");
         setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/sw_select_results","PNG").scaled(128,128));

         QGridLayout* layout = new QGridLayout(this);

         mButAll = new QPushButton("Wave data for all nets", this);
         layout->addWidget(mButAll,0,0);
         mButNone = new QPushButton("Clear selection", this);
         layout->addWidget(mButNone,0,2);

         mTableView = new QTableView(this);
         mProxyModel = new QSortFilterProxyModel(this);
         mWaveModel = new WaveSelectionTable(mTableView);
         mProxyModel->setSourceModel(mWaveModel);
         mTableView->setModel(mProxyModel);
         mTableView->setSortingEnabled(true);
         mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
         mTableView->setSelectionMode(QAbstractItemView::MultiSelection);
         QHeaderView* hv = mTableView->horizontalHeader();
         hv->setSectionResizeMode(0,QHeaderView::Interactive);
         hv->setSectionResizeMode(1,QHeaderView::Stretch);
         hv->setSectionResizeMode(2,QHeaderView::Interactive);
         mTableView->setColumnWidth(0,36);
         mTableView->setColumnWidth(1,256);
         mTableView->setColumnWidth(2,36);
         connect(mButAll,&QPushButton::clicked,mTableView,&QTableView::selectAll);
         connect(mButNone,&QPushButton::clicked,mTableView,&QTableView::clearSelection);

         layout->addWidget(mTableView,1,0,1,3);
    }

    bool PageLoadResults::validatePage()
    {
        QList<QModelIndex> selIndexList;
        for (QModelIndex proxyInx : mTableView->selectionModel()->selectedIndexes())
        {
            selIndexList.append(mProxyModel->mapToSource(proxyInx));
        }
        if (!selIndexList.isEmpty())
            mWaveWidget->addSelectedResults(mWaveModel->entryMap(selIndexList));
        return true;
    }

    void PageLoadResults::initializePage()
    {
        mWaveModel->setEntryMap(mWaveWidget->addableEntries());
    }
}
