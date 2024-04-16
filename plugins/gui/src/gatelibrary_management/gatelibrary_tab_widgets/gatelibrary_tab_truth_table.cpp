#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_truth_table.h"
#include <QLineEdit>

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{

    GateLibraryTabTruthTable::GateLibraryTabTruthTable(QWidget* parent)
        : GateLibraryTabInterface(parent), mGateType(nullptr)
    {
        mLayout = new QGridLayout(this);


        mLutInit = new GatelibraryLutInit(this);
        connect(mLutInit, &GatelibraryLutInit::initValueChanged, this, &GateLibraryTabTruthTable::handleLutInitValueChanged);
        mLayout->addWidget(mLutInit);
        mLutInit->hide();

        mTableWidget = new QTableWidget();

        mDisclaimer = new QLabel(this);
        mDisclaimer->setAlignment(Qt::AlignCenter);
        mLayout->addWidget(mDisclaimer);
        mDisclaimer->hide();
        mLayout->addWidget(mTableWidget);

        //mHeaderView = new QHeaderView(Qt::Horizontal);
        //mTableWidget->setHorizontalHeader(mHeaderView);
    }

    void GateLibraryTabTruthTable::setTableSize()
    {
        mTableWidget->setColumnCount(getColumnNumber());
        mTableWidget->setRowCount(getRowNumber());
        mTableWidget->verticalHeader()->hide();

        QStringList header;
        std::vector<std::string> inputs = mGateType->get_input_pin_names();
        std::vector<std::string> outputs = mGateType->get_output_pin_names();
        for (const std::string& inputPinName : inputs)
            header << "Input\n" + QString::fromStdString(inputPinName);
        for (const std::string& outputPinName : outputs)
            header << "Output\n" + QString::fromStdString(outputPinName);
        mTableWidget->setHorizontalHeaderLabels(header);
    }

    bool GateLibraryTabTruthTable::updateFlipFlop()
    {
        QHash<QString,int> inputColHash;
        QHash<QString,int> outputColHash;
        QHash<QString,QPair<int,int>> stateHash;
        QStringList states;
        QStringList header;
        QSet<QString> requiredForOutput;
        int columnNumber = 0;
        int icolClock = -1;
        int icolSet = -1;
        int icolReset = -1;
        int rsRows = 0;

        StateComponent* scomp = mGateType->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
        if (!scomp) return false;
        FFComponent* ffcomp = mGateType->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });
        if (!ffcomp) return false;

        for (auto & [out,bf] : mGateType->get_boolean_functions())
            for (const std::string& var : bf.get_variable_names())
                requiredForOutput << QString::fromStdString(var);

        auto resNextState = ffcomp->get_next_state_function().compute_truth_table();
        QStringList nextStateInputs;
        for (const std::string& nsi : ffcomp->get_next_state_function().get_variable_names())
            nextStateInputs << QString::fromStdString(nsi);

        if (!resNextState.is_ok() || resNextState.get().size() != 1) return false;

        // input pins
        for (GatePin* pin : mGateType->get_input_pins())
        {
             QString pinName = QString::fromStdString(pin->get_name());
             header << "input\n" + QString::fromStdString(enum_to_string<PinType>(pin->get_type())) + "\n" + pinName;
             switch (pin->get_type()) {
             case PinType::clock:
                 icolClock = columnNumber;
                 break;
             case PinType::set:
                 icolSet = columnNumber;
                 ++rsRows;
                 break;
             case PinType::reset:
                 icolReset = columnNumber;
                 ++rsRows;
                 break;
             default:
                 break;
             }
             inputColHash[pinName] = columnNumber++;
        }

        if (rsRows == 2) rsRows = 3;

        // internal states
        QStringList tempQ;

        for (int i=0; i<2; i++)
        {
            QString qx  = QString::fromStdString(i ? scomp->get_neg_state_identifier()
                                                    : scomp->get_state_identifier());
            if (requiredForOutput.contains(qx))
                tempQ << qx;
        }

        for (const QString& qx : tempQ)
        {
            header << "last\n" + qx;
            stateHash[qx] = QPair<int,int>(columnNumber,columnNumber+tempQ.size());
            inputColHash[qx] = columnNumber++;
        }
        for (const QString& qx : tempQ)
        {
            header << "next\n" + qx;
            ++columnNumber;
        }

        Q_ASSERT(!stateHash.isEmpty());

        // output pins
        for (GatePin* pin : mGateType->get_output_pins())
        {
             QString pinName = QString::fromStdString(pin->get_name());
             header << "output\n" + QString::fromStdString(enum_to_string<PinType>(pin->get_type())) + "\n" + pinName;
             outputColHash[pinName] = columnNumber++;
        }
        mTableWidget->setColumnCount(columnNumber);

        auto tabNextState = resNextState.get().at(0);
        mTableWidget->setRowCount(tabNextState.size()+2+rsRows);
        mTableWidget->setHorizontalHeaderLabels(header);

        int irow = 0;

        if (icolSet >= 0)
        {
            for (int icol = 0; icol < inputColHash.size(); icol++)
            {
                if (icol == icolSet)
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("H")); // TODO check !S
                else if (icol == icolReset)
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("L"));
                else
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("X"));
            }
            mTableWidget->setItem(irow, stateHash.begin()->first, new QTableWidgetItem("X"));

            QString qx = "H";
            mTableWidget->setItem(irow, stateHash.begin()->second, new QTableWidgetItem(qx));
            for (int icol : outputColHash.values())
                mTableWidget->setItem(irow, icol, new QTableWidgetItem(qx));

            ++irow;
        }

        if (icolReset >= 0)
        {
            for (int icol = 0; icol < inputColHash.size(); icol++)
            {
                if (icol == icolSet)
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("L"));
                else if (icol == icolReset)
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("H"));// TODO check !R
                else
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("X"));
            }
            mTableWidget->setItem(irow, stateHash.begin()->first, new QTableWidgetItem("X"));

            QString qx = "L";
            mTableWidget->setItem(irow, stateHash.begin()->second, new QTableWidgetItem(qx));
            for (int icol : outputColHash.values())
                mTableWidget->setItem(irow, icol, new QTableWidgetItem(qx));
            ++irow;
        }

        if (icolSet >= 0 && icolReset >= 0)
        {
            for (int icol = 0; icol < inputColHash.size(); icol++)
            {
                if (icol == icolSet)
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("H"));
                else if (icol == icolReset)
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("H"));// TODO check !R
                else
                    mTableWidget->setItem(irow, icol, new QTableWidgetItem("X"));
            }
            mTableWidget->setItem(irow, stateHash.begin()->first, new QTableWidgetItem("X"));

            QString qx = QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(ffcomp->get_async_set_reset_behavior().first));
            mTableWidget->setItem(irow, stateHash.begin()->second, new QTableWidgetItem(qx));
            for (int icol : outputColHash.values())
                mTableWidget->setItem(irow, icol, new QTableWidgetItem(qx));
            ++irow;
        }

        for (const BooleanFunction::Value& bfVal : resNextState.get().at(0))
        {
            mTableWidget->setItem(irow, stateHash.begin()->first, new QTableWidgetItem("X"));
            for (int nsiCol = 0; nsiCol < nextStateInputs.size(); nsiCol++)
            {
                int icol = inputColHash.value(nextStateInputs.at(nsiCol),-1);
                Q_ASSERT(icol >= 0);
                QString cellVal(irow & (1<<nsiCol) ? "H" : "L");
                mTableWidget->setItem(irow, icol, new QTableWidgetItem(cellVal));
            }
            setCellValue(irow,stateHash.begin()->second,bfVal);
            if (icolClock >= 0)
                mTableWidget->setItem(irow, icolClock, new QTableWidgetItem("UP")); // arrow up = QString(QChar(0x2191))
            if (icolReset >= 0)
                mTableWidget->setItem(irow, icolReset, new QTableWidgetItem("L"));
            if (icolSet >= 0)
                mTableWidget->setItem(irow, icolSet, new QTableWidgetItem("L"));
            for (int icol : outputColHash.values())
                setCellValue(irow,icol,bfVal);
            ++irow;
        }

        for (int ihold = 0; ihold < 2; ihold++)
        {
            for (int nsiCol = 0; nsiCol < nextStateInputs.size(); nsiCol++)
            {
                int icol = inputColHash.value(nextStateInputs.at(nsiCol),-1);
                Q_ASSERT(icol >= 0);
                mTableWidget->setItem(irow, icol, new QTableWidgetItem("X"));
            }
            if (icolClock >= 0)
                mTableWidget->setItem(irow, icolClock, new QTableWidgetItem("-"));
            if (icolReset >= 0)
                mTableWidget->setItem(irow, icolReset, new QTableWidgetItem("L"));
            if (icolSet >= 0)
                mTableWidget->setItem(irow, icolSet, new QTableWidgetItem("L"));

            QString qx = ihold ? "H" : "L";
            mTableWidget->setItem(irow, stateHash.begin()->first, new QTableWidgetItem(qx));
            mTableWidget->setItem(irow, stateHash.begin()->second, new QTableWidgetItem(qx));
            for (int icol : outputColHash.values())
                mTableWidget->setItem(irow, icol, new QTableWidgetItem(qx));
            ++irow;
        }

        return true;
    }

    void GateLibraryTabTruthTable::updateLookupTable()
    {
        setTableSize();
        mLutInit->update(mGateType);
    }

    void GateLibraryTabTruthTable::update(GateType* gt)
    {
        mGateType = gt;

        if (mGateType && mGateType->has_component_of_type(GateTypeComponent::ComponentType::lut))
        {
            updateLookupTable();
            mLutInit->show();
            mTableWidget->show();
            mDisclaimer->hide();
            return;
        }

        mLutInit->hide();

        if (mGateType && mGateType->has_component_of_type(GateTypeComponent::ComponentType::ff))
        {
            if (updateFlipFlop())
            {
                mTableWidget->show();
                mDisclaimer->hide();
                return;
            }
        }

        if(mGateType)
        {
            if (getColumnNumber()-1 > 8)
            {
                mTableWidget->hide();
                mDisclaimer->setText(QString("Cannot calculate truth table\nfor gate with %1 input pins\nand %2 output pins")
                                     .arg(mGateType->get_input_pins().size())
                                          .arg(mGateType->get_output_pins().size()));
                mDisclaimer->show();
            }
            else
            {
                bool undefinedResult = false;

                setTableSize();

                std::vector<std::string> inputs = mGateType->get_input_pin_names();
                BooleanFunction boolFunc = mGateType->get_boolean_function();
                auto truthTable = boolFunc.compute_truth_table(inputs, false).get().at(0);

                for (uint irow = 0; irow < truthTable.size(); irow++)
                {
                    //iterate from 0..0 to 1..1
                    for (uint icol = 0; icol < mGateType->get_input_pins().size(); icol++)
                    {
                        u8 inputBit = u8((irow >> icol) & 1);
                        QTableWidgetItem* item = new QTableWidgetItem(inputBit ? "H" : "L");
                        mTableWidget->setItem(irow, icol, item);
                    }

                    //fill the output columns
                    for (uint icol = mGateType->get_input_pins().size(); icol < mGateType->get_output_pins().size()+mGateType->get_input_pins().size(); icol++)
                    {
                        if (!setCellValue(irow,icol,truthTable[irow]))
                            undefinedResult = true;
                    }
                }
                if (undefinedResult)
                {
                    mDisclaimer->setText("Truth table calculation\nfor gate type <" + QString::fromStdString(mGateType->get_name()) + ">\nnot implemented so far");
                    mDisclaimer->show();
                    mTableWidget->hide();
                }
                else
                {
                    mDisclaimer->hide();
                    mTableWidget->show();
                }
            }
        }
        else
        {
            mDisclaimer->setText("No gate type selected");
            mDisclaimer->show();
            mTableWidget->hide();
        }
    }

    bool GateLibraryTabTruthTable::setCellValue(int irow, int icol, BooleanFunction::Value bfval)
    {

        if (bfval == BooleanFunction::Value::ZERO)
        {
            QTableWidgetItem* item = new QTableWidgetItem("L");
            mTableWidget->setItem(irow, icol, item);
        }
        else if (bfval == BooleanFunction::Value::ONE)
        {
            QTableWidgetItem* item = new QTableWidgetItem("H");
            mTableWidget->setItem(irow, icol, item);
        }
        else if (bfval == BooleanFunction::Value::Z)
        {
            QTableWidgetItem* item = new QTableWidgetItem("Z");
            mTableWidget->setItem(irow, icol, item);
        }
        else
        {
            QTableWidgetItem* item = new QTableWidgetItem("X");
            mTableWidget->setItem(irow, icol, item);
            return false;
        }
        return true;
    }


    void GateLibraryTabTruthTable::handleLutInitValueChanged(u64 initValue)
    {
        setTableSize();

        for (int irow = 0; irow < mTableWidget->rowCount(); irow++)
        {
            //iterate from 0..0 to 1..1
            for (uint icol = 0; icol < mGateType->get_input_pins().size(); icol++)
            {
                u8 inputBit = u8((irow >> icol) & 1);
                QTableWidgetItem* item = new QTableWidgetItem(inputBit ? "H" : "L");
                mTableWidget->setItem(irow, icol, item);
            }

            //fill the output columns
            for (uint icol = mGateType->get_input_pins().size(); icol < mGateType->get_output_pins().size()+mGateType->get_input_pins().size(); icol++)
            {
                u64 mask = 1;
                mask <<= irow;
                if (initValue & mask)
                {
                    QTableWidgetItem* item = new QTableWidgetItem("H");
                    mTableWidget->setItem(irow, icol, item);
                }
                else
                {
                    QTableWidgetItem* item = new QTableWidgetItem("L");
                    mTableWidget->setItem(irow, icol, item);
                }
            }
        }
        mDisclaimer->hide();
        mTableWidget->show();
    }

    int GateLibraryTabTruthTable::getRowNumber() const
    {
        return pow(2, mGateType->get_input_pins().size()); //iterate from 0..0 to 2^n
    }

    int GateLibraryTabTruthTable::getColumnNumber() const
    {
        if(mGateType)
        {
            return mGateType->get_input_pins().size()+mGateType->get_output_pins().size();
        }
        return 0;
    }

    GatelibraryLutInit::GatelibraryLutInit(QWidget* parent)
        : GatelibraryComponentFrame("LUT init hex value", parent)
    {
        mEdit = new QLineEdit(this);
        connect(mEdit,&QLineEdit::textChanged,this,&GatelibraryLutInit::handleTextChanged);
        mLayout->addWidget(mEdit);
        setMaximumHeight(64);
    }

    void GatelibraryLutInit::handleTextChanged(const QString& txt)
    {
        bool ok;
        u64 val = txt.toULongLong(&ok,16);
        if (ok) Q_EMIT initValueChanged(val);
    }

    void GatelibraryLutInit::update(GateType* gt)
    {
        if (gt && gt->has_component_of_type(GateTypeComponent::ComponentType::lut))
        {
            auto lut = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });

            if (lut != nullptr)
            {
                mEdit->setText("1");
                Q_EMIT initValueChanged(1);
                show();
            }
            else
                hide();
        }
        else
            hide();

    }
}
