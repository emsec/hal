#include "logic_evaluator/logic_evaluator_dialog.h"
#include "logic_evaluator/logic_evaluator_pingroup.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/module_model/module_model.h"
#include "logic_evaluator/logic_evaluator_select_gates.h"
#include "logic_evaluator/logic_evaluator_truthtable.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QStyle>
#include <QSet>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QDebug>
#include <QSpacerItem>

namespace hal {

    const char* LOGIC_EVALUATOR_GET  = "logic_evaluator_get";
    const char* LOGIC_EVALUATOR_SET  = "logic_evaluator_set";
    const char* LOGIC_EVALUATOR_CALC = "logic_evaluator_calc";
    const char* COMPILER = "gcc";

    LogicEvaluatorDialog::LogicEvaluatorDialog(const std::vector<Gate *>& gates, bool skipCompile, QWidget *parent)
        : QDialog(parent), mGates(gates), mSimulationInput(new SimulationInput), mActionCompile(nullptr), mActionIndicate(nullptr)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle(QString("Logic Evaluator %1 Gates").arg(gates.size()));

        if (gates.empty())
        {
            log_warning("logic_evaluator", "No eligible gates selected for logic evaluator, window will close");
            close();
        }

        mSimulationInput->add_gates(gates);
        mSimulationInput->compute_net_groups();

        calculateEvaluationOrder();
        QHBoxLayout* topLayout = new QHBoxLayout(this);
        topLayout->setSpacing(0);
        QVBoxLayout* inpLayout = new QVBoxLayout;
        inpLayout->setSpacing(4);
        QVBoxLayout* outLayout = new QVBoxLayout;
        outLayout->setSpacing(4);

        // input pingroups
        std::unordered_set<const Net*> inputNets =  mSimulationInput->get_input_nets();
        for (SimulationInput::NetGroup grp : mSimulationInput->get_net_groups())
        {
            if (!grp.is_input) continue;
            for (const Net* n : grp.get_nets())
            {
                auto it = inputNets.find(n);
                if (it != inputNets.end()) inputNets.erase(it);
            }
            LogicEvaluatorPingroup* lep = new LogicEvaluatorPingroup(grp.get_nets(), false, QString::fromStdString(grp.get_name()), this);
            connect(lep, &LogicEvaluatorPingroup::triggerRecalc, this, &LogicEvaluatorDialog::recalc);
            mInputs.append(lep);
            inpLayout->addWidget(lep);
        }

        // input single pins
        for (const Net* n : inputNets)
        {
            LogicEvaluatorPingroup* lep = new LogicEvaluatorPingroup(n, false, this);
            connect(lep, &LogicEvaluatorPingroup::triggerRecalc, this, &LogicEvaluatorDialog::recalc);
            mInputs.append(lep);
            inpLayout->addWidget(lep);
        }

        // output pingroups
        std::unordered_set<const Net*> outputNets(mSimulationInput->get_output_nets().begin(),mSimulationInput->get_output_nets().end());
        for (SimulationInput::NetGroup grp : mSimulationInput->get_net_groups())
        {
            if (grp.is_input) continue;
            bool isOutputGroup = true;

            std::vector<const Net*> temp;
            for (const Net* n : grp.get_nets())
            {
                auto it = outputNets.find(n);
                if (it == outputNets.end())
                {
                    isOutputGroup = false;
                    break;
                }
                else
                {
                    outputNets.erase(it);
                    temp.push_back(n);
                }
            }
            if (!isOutputGroup)
            {
                for (const Net* n : temp)
                    outputNets.insert(n);
                continue;
            }
            LogicEvaluatorPingroup* lep = new LogicEvaluatorPingroup(grp.get_nets(), true, QString::fromStdString(grp.get_name()), this);
            connect(lep, &LogicEvaluatorPingroup::triggerRecalc, this, &LogicEvaluatorDialog::recalc);
            mOutputs.append(lep);
            outLayout->addWidget(lep);
        }

        // output single pins
        for (const Net* n : outputNets)
        {
            QStringList netName;
            netName.append(QString::fromStdString(n->get_name()));
            LogicEvaluatorPingroup* lep = new LogicEvaluatorPingroup(n, true, this);
            connect(lep, &LogicEvaluatorPingroup::triggerRecalc, this, &LogicEvaluatorDialog::recalc);
            mOutputs.append(lep);
            outLayout->addWidget(lep);
        }

        QTreeView* tview = new QTreeView(this);
        ModuleModel* tmodel = new ModuleModel(this);
        tmodel->populateFromGatelist(gates);
        tview->setModel(tmodel);
        tview->expandAll();
        tview->setColumnWidth(0,250);
        tview->setColumnWidth(1,40);
        tview->setColumnWidth(2,110);

        mMenuBar = new QMenuBar(this);
        QMenu* options = mMenuBar->addMenu("Options");
        mActionCompile = options->addAction("Run compiled logic");
        connect(mActionCompile, &QAction::toggled, this, &LogicEvaluatorDialog::handleCompiledToggled);
        mActionCompile->setCheckable(true);
        mActionCompile->setChecked(false);
        mActionIndicate = options->addAction("Show in graphic view");
        connect(mActionIndicate, &QAction::toggled, this, &LogicEvaluatorDialog::handleIndicateToggled);
        mActionIndicate->setCheckable(true);
        QAction* relaunch = mMenuBar->addAction("Relaunch");
        connect(relaunch, &QAction::triggered, this, &LogicEvaluatorDialog::handleRelaunchTriggered);
        QAction* ttable = mMenuBar->addAction("Truth Table");
        connect(ttable, &QAction::triggered, this, &LogicEvaluatorDialog::handleTruthtableTriggered);

        inpLayout->addStretch();
        outLayout->addStretch();
        topLayout->addLayout(inpLayout);
        topLayout->addWidget(tview);
        topLayout->addLayout(outLayout);
        topLayout->setMenuBar(mMenuBar);

        tview->setMinimumWidth(400);

        if (!skipCompile)
        {
            mActionCompile->setChecked(true);
        }

        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        recalc();
    }

    LogicEvaluatorDialog::~LogicEvaluatorDialog()
    {
        delete mSimulationInput;
    }

    void LogicEvaluatorDialog::handleRelaunchTriggered()
    {
        LogicEvaluatorSelectGates lesg(mGates, this);
        if (lesg.exec() == QDialog::Accepted)
            lower();
    }

    void LogicEvaluatorDialog::handleTruthtableTriggered()
    {
        if (!mSharedLib.handle) return;
        if (!mTruthtable)
        {
            QList<const Net*> inpList;
            QList<const Net*> outList;
            for (const LogicEvaluatorPingroup* lepg : mInputs)
                for (int i=lepg->size()-1; i>=0; i--)
                    inpList.append(lepg->getValue(i).first);
            for (const LogicEvaluatorPingroup* lepg : mOutputs)
                for (int i=lepg->size()-1; i>=0; i--)
                    outList.append(lepg->getValue(i).first);
            if (inpList.isEmpty() || outList.isEmpty()) return;
            if (inpList.size() > 10)
            {
                log_warning("logic_evaluator", "Cannot generate truth table for {} logic inputs.", inpList.size());
                return;
            }
            mTruthtable = new LogicEvaluatorTruthtableModel(inpList,outList,this);

            int maxInput = 1 << inpList.size();

            for (int inputVal = 0; inputVal < maxInput; inputVal++)
            {
                QList<int> values;
                int mask = 1;
                for (const Net* n : inpList)
                {
                    int bitVal = (inputVal&mask) ? 1 : 0;
                    values.append(bitVal);
                    mSharedLib.set(mExternalArrayIndex[n], bitVal);
                    mask <<= 1;
                }
                mSharedLib.calc();
                for (const Net* n : outList)
                {
                    int bitVal = mSharedLib.get(mExternalArrayIndex[n]);
                    values.append(bitVal);
                }
                mTruthtable->addColumn(new LogicEvaluatorTruthtableColumn(inpList.size()+outList.size(),values));
            }
        }
        if (!mTruthtable) return;

        LogicEvaluatorTruthtable lett(mTruthtable, this);
        if (lett.exec() == QDialog::Accepted)
        {
            QMap<const Net*,int> vals = lett.selectedColumn();
            for (auto it = vals.constBegin(); it != vals.constEnd(); ++it)
            {
                const Net* n = it.key();
                BooleanFunction::Value bv = it.value() ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO;
                for (LogicEvaluatorPingroup* lepg : mInputs)
                    lepg->setValue(n,bv);
            }
        }
    }

    void LogicEvaluatorDialog::handleCompiledToggled(bool checked)
    {
        if (checked && !mSharedLib.handle)
        {
            compile();
            if (!mSharedLib.handle)
                mActionCompile->setChecked(false);
        }
    }

    void LogicEvaluatorDialog::handleIndicateToggled(bool checked)
    {
        if (checked)
            recalc();
        else
            omitNetlistVisualization();
    }

    void LogicEvaluatorDialog::SharedLibHandle::close()
    {
        if (handle)
        {
            dlclose(handle);
            handle = nullptr;
        }
        QFile::remove(fnSharedLib);
    }

    bool LogicEvaluatorDialog::compile()
    {
        // get input signals
        QMap<QString,int> pinVars;
        QString codeEvalFunction;
        mExternalArrayIndex.clear();

        for (const LogicEvaluatorPingroup* lepg : mInputs)
        {
            for (int i=0; i<lepg->size(); i++)
            {
                QPair<const Net*, BooleanFunction::Value> v = lepg->getValue(i);
                const Net* n = v.first;
                int sz = mExternalArrayIndex.size();
                codeEvalFunction += QString("  // input[%1] - Net %2 <%3>;\n").arg(sz).arg(n->get_id()).arg(n->get_name().c_str());
                mExternalArrayIndex[n] = sz;
            }
        }

        // propagate by gates
        for (const Gate* g : mEvaluationOrder)
        {
            for (const GatePin* gp : g->get_type()->get_input_pins())
            {
                QString pinName = QString::fromStdString(gp->get_name());
                pinVars[pinName]++;
                const Net* n = g->get_fan_in_net(gp);
                int inx = mExternalArrayIndex.value(n,-1);
                if (inx < 0) return false;
                codeEvalFunction += QString("  %1 = logic_evaluator_signals[%2];\n").arg(pinName).arg(inx);
            }
            for (const GatePin* gp : g->get_type()->get_output_pins())
            {
                QString pinName = QString::fromStdString(gp->get_name());
                pinVars[pinName]++;
                const Net* n = g->get_fan_out_net(gp);
                if (!mExternalArrayIndex.contains(n))
                {
                    int sz = mExternalArrayIndex.size();
                    mExternalArrayIndex[n] = sz;
                }
                codeEvalFunction += QString("  %1 = %2;\n").arg(pinName).arg(QString::fromStdString(g->get_boolean_function(gp).to_string()));
                int inx = mExternalArrayIndex.value(n,-1);
                if (inx < 0) return false;
                codeEvalFunction += QString("  logic_evaluator_signals[%1] = %2;\n").arg(inx).arg(pinName);
            }
        }

        for (LogicEvaluatorPingroup* lepg : mOutputs)
        {
            int k = lepg->size();
            for (int i=0; i<k; i++)
            {
                const Net* n = lepg->getValue(i).first;
                int inx = mExternalArrayIndex.value(n,-1);
                if (inx < 0) return false;
                codeEvalFunction += QString("  // output[%1] - Net %2 <%3>;\n").arg(inx).arg(n->get_id()).arg(n->get_name().c_str());
            }
        }
        QString ccode;
        ccode += QString("int logic_evaluator_signals[%1];\n\n").arg(mExternalArrayIndex.size());
        ccode += "void " + QString(LOGIC_EVALUATOR_SET);
        ccode += "(int inx, int val) {\n"
                "  logic_evaluator_signals[inx] = val;\n"
                "}\n\n";
        ccode += "int " + QString(LOGIC_EVALUATOR_GET);
        ccode += "(int inx) {\n"
                "  return logic_evaluator_signals[inx];\n"
                "}\n\n";
        ccode += "void " + QString(LOGIC_EVALUATOR_CALC);
        ccode += "() {\n";
        for (QString var : pinVars.keys())
        {
            ccode += QString("  int %1;\n").arg(var);
        }
        ccode += codeEvalFunction + "\n}\n";

        // write code to file
        QTemporaryFile ftemp(QDir().temp().absoluteFilePath("logic_evaluator_shared_lib_XXXXXX.c"));
        ftemp.setAutoRemove(true);
        if (!ftemp.open())
            return false;
        mSharedLib.fnSharedLib = ftemp.fileName();
        mSharedLib.fnSharedLib.replace(QRegularExpression("\.c$"), QString(".%1").arg(LIBRARY_FILE_EXTENSION));
        ftemp.write(ccode.toUtf8());
        ftemp.close();

        // compile code
        QProcess proc(this);
        QStringList args;
        args <<  "-shared" << "-Wall" << "-Werror" << "-fpic" << "-o" << mSharedLib.fnSharedLib << ftemp.fileName();
        if (proc.execute(COMPILER, args) < 0)
        {
            log_warning("logic_evaluator", "Failed to run compiler '{}', cannot compile logic.", COMPILER);
            return false;
        }
        proc.waitForStarted();
        proc.waitForFinished();

        if (proc.exitCode() || proc.exitStatus() != QProcess::NormalExit)
        {
            log_warning("logic_evaluator", "Failed to compile '{}', stdout: '{}', stderr: '{}", ftemp.fileName().toStdString(), proc.readAllStandardOutput().constData(), proc.readAllStandardError().constData());
            return false;
        }

        // load shared libraray

        mSharedLib.handle = dlopen(mSharedLib.fnSharedLib.toUtf8().constData(), RTLD_LAZY);
        if (!mSharedLib.handle)
        {
            log_warning("logic_evaluator", "Failed to load shared library '{}' dlerror: '{}'.", mSharedLib.fnSharedLib.toStdString(), dlerror());
            return false;
        }

        // reset errors
        dlerror();

        mSharedLib.get = (int(*)(int)) dlsym(mSharedLib.handle, LOGIC_EVALUATOR_GET);
        const char* dlsymError = dlerror();
        if (dlsymError) {
            log_warning("logic_evaluator", "Cannot resolve symbol '{}' in shared library, dlerror: '{}'.", LOGIC_EVALUATOR_GET, dlsymError);
            mSharedLib.close();
            return false;
        }

        mSharedLib.set = (void(*)(int,int)) dlsym(mSharedLib.handle, LOGIC_EVALUATOR_SET);
        dlsymError = dlerror();
        if (dlsymError) {
            log_warning("logic_evaluator", "Cannot resolve symbol '{}' in shared library, dlerror: '{}'.", LOGIC_EVALUATOR_SET, dlsymError);
            mSharedLib.close();
            return false;
        }

        mSharedLib.calc = (void(*)(void)) dlsym(mSharedLib.handle, LOGIC_EVALUATOR_CALC);
        dlsymError = dlerror();
        if (dlsymError) {
            log_warning("logic_evaluator", "Cannot resolve symbol '{}' in shared library, dlerror: '{}'.", LOGIC_EVALUATOR_CALC, dlsymError);
            mSharedLib.close();
            return false;
        }

        log_info("logic_evaluator", "Temporary shared library '{}' successfully build and loaded.", mSharedLib.fnSharedLib.toStdString());

        return true;
    }

    void LogicEvaluatorDialog::recalc()
    {
        mSignals.clear();
        if (mActionCompile->isChecked() && mSharedLib.handle)
            recalcCompiled();
        else
            recalcInterpreted();

        for (LogicEvaluatorPingroup* lepg : mOutputs)
        {
            int k = lepg->size();
            for (int i=0; i<k; i++)
            {
                const Net* n = lepg->getValue(i).first;
                lepg->setValue(n, mSignals.value(n, BooleanFunction::Value::X));
            }
        }

        if (mActionIndicate->isChecked())
            visualizeResultsInNetlist();
    }

    void LogicEvaluatorDialog::recalcCompiled()
    {
        for (const LogicEvaluatorPingroup* lepg : mInputs)
        {
            for (int i=0; i<lepg->size(); i++)
            {
                QPair<const Net*, BooleanFunction::Value> v = lepg->getValue(i);
                const Net* n = v.first;
                int inx = mExternalArrayIndex.value(n,-1);
                if (inx < 0)
                {
                    log_warning("logic_evaluator", "No shared library index for input net id={} '{}'.", n->get_id(), n->get_name());
                    return;
                }
                mSharedLib.set(inx, (int) v.second);
            }
        }

        mSharedLib.calc();

        for (auto it=mExternalArrayIndex.begin(); it!=mExternalArrayIndex.end(); ++it)
        {
            const Net* n = it.key();
            int val = mSharedLib.get(it.value());
            mSignals[n] = (BooleanFunction::Value) val;
        }
    }

    void LogicEvaluatorDialog::recalcInterpreted()
    {
        // get input signals
        for (const LogicEvaluatorPingroup* lepg : mInputs)
        {
            for (int i=0; i<lepg->size(); i++)
            {
                QPair<const Net*, BooleanFunction::Value> v = lepg->getValue(i);
                mSignals[v.first] = v.second;
            }
        }

        // propagate by gates
        for (const Gate* g : mEvaluationOrder)
        {
            std::unordered_map<std::string, BooleanFunction::Value> gateSignals;
            for (const GatePin* gp : g->get_type()->get_input_pins())
            {
                const Net* n = g->get_fan_in_net(gp);
                gateSignals[gp->get_name()] = mSignals.value(n, BooleanFunction::Value::X);
            }
            for (const GatePin* gp : g->get_type()->get_output_pins())
            {
                const Net* n = g->get_fan_out_net(gp);
                auto res = g->get_boolean_function(gp).evaluate(gateSignals);
                if (res.is_ok())
                    mSignals[n] = res.get();
                else
                    log_warning("logic_evaluator", "Failed to evaluate boolean function '{}'.", g->get_boolean_function(gp).to_string());
            }
        }
    }

    void LogicEvaluatorDialog::calculateEvaluationOrder()
    {
        mEvaluationOrder.clear();
        QHash<const Gate*, std::vector<Net*> > undeterminedInput;

        // setup hash, declare all inpus as undetermined;
        for (const Gate* g: mSimulationInput->get_gates())
        {
            undeterminedInput.insert(g, g->get_fan_in_nets());
        }

        std::unordered_set<const Net*> inputSignals = mSimulationInput->get_input_nets();

        int resolved = 1;
        while (!undeterminedInput.isEmpty() && resolved)
        {
            resolved = 0;
            std::unordered_set<const Net*> outputSignals;
            auto it = undeterminedInput.begin();
            while (it != undeterminedInput.end())
            {
                auto jt = it.value().begin();
                while (jt != it.value().end())
                {
                    if (inputSignals.find(*jt) == inputSignals.end())
                        ++jt;
                    else
                        jt = it.value().erase(jt);
                }

                // has no more undetermined inputs ?
                if (it.value().empty())
                {
                    mEvaluationOrder.append(it.key());
                    for (const Net* n : it.key()->get_fan_out_nets())
                        outputSignals.insert(n);
                    it = undeterminedInput.erase(it);
                    ++resolved;
                }
                else
                {
                    ++it;
                }
            }
            inputSignals = outputSignals;
        }
        if (!undeterminedInput.isEmpty())
        {
            std::string leftover;
            for (const Gate* g : undeterminedInput.keys())
                leftover += " [" + std::to_string(g->get_id()) + ',' + g->get_name() + ']';
            log_warning("logic_evaluator", "Cannot determine evaluation order, {} gate(s) left with undetermined input: {}.", undeterminedInput.size(), leftover);
        }
    }

    void LogicEvaluatorDialog::visualizeResultsInNetlist()
    {
        GroupingTableModel* gtm = gContentManager->getGroupingManagerWidget()->getModel();
        const char* color[] = {"#707071", "#102080", "#802010" };
        static const char* grpNames[3] = {"x state", "0 state", "1 state"};
        Grouping* grp[3];
        for (int i=0; i<3; i++)
        {
            grp[i] = gtm->groupingByName(grpNames[i]);
            if (!grp[i])
            {
                grp[i] = gNetlist->create_grouping(grpNames[i]);
                gtm->recolorGrouping(grp[i]->get_id(),QColor(color[i]));
            }
        }

        for (auto it = mSignals.constBegin(); it != mSignals.constEnd(); ++it)
        {
            int grpIndex = 1 + (int) it.value();
            Q_ASSERT(grpIndex >= 0 && grpIndex <= 2);
            grp[grpIndex]->assign_net(const_cast<Net*>(it.key()),true);
        }
    }

    void LogicEvaluatorDialog::omitNetlistVisualization()
    {
        GroupingTableModel* gtm = gContentManager->getGroupingManagerWidget()->getModel();
        static const char* grpNames[3] = {"x state", "0 state", "1 state"};
        for (int i=0; i<3; i++)
        {
            Grouping* grp = gtm->groupingByName(grpNames[i]);
            if (grp) gNetlist->delete_grouping(grp);
        }
    }

}
