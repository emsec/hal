
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_truth_table.h"

#include "gui/gui_globals.h"


namespace hal
{

    GateLibraryTabTruthTable::GateLibraryTabTruthTable(QWidget* parent) : GateLibraryTabInterface(parent)
    {
        mLayout = new QGridLayout(this);
        mTableWidget = new QTableWidget();

        mDisclaimer = new QLabel(this);
        mDisclaimer->setAlignment(Qt::AlignCenter);
        mLayout->addWidget(mDisclaimer);
        mDisclaimer->hide();
        mLayout->addWidget(mTableWidget);

        //mHeaderView = new QHeaderView(Qt::Horizontal);
        //mTableWidget->setHorizontalHeader(mHeaderView);
    }

    void GateLibraryTabTruthTable::update(GateType* gate)
    {
        if(gate)
        {
            bool undefinedResult = false;

            if (getColumnNumber(gate)-1 > 8)
            {
                mTableWidget->hide();
                mDisclaimer->setText(QString("Cannot calculate truth table\nfor gate with %1 input pins\nand %2 output pins")
                                     .arg(gate->get_input_pins().size())
                                          .arg(gate->get_output_pins().size()));
                mDisclaimer->show();
                return;
            }

            mTableWidget->show();

            mTableWidget->setColumnCount(getColumnNumber(gate));
            mTableWidget->setRowCount(getRowNumber(gate));
            mTableWidget->verticalHeader()->hide();

            BooleanFunction boolFunc = gate->get_boolean_function();
            QStringList header;
            std::vector<std::string> inputs = gate->get_input_pin_names();
            std::vector<std::string> outputs = gate->get_output_pin_names();
            for (const std::string& inputPinName : inputs)
                header << "Input\n" + QString::fromStdString(inputPinName);
            for (const std::string& outputPinName : outputs)
                header << "Output\n" + QString::fromStdString(outputPinName);
            mTableWidget->setHorizontalHeaderLabels(header);

            auto truthTable = boolFunc.compute_truth_table(inputs, false).get().at(0);

            for (uint irow = 0; irow < truthTable.size(); irow++)
            {
                //iterate from 0..0 to 1..1
                for (uint icol = 0; icol < gate->get_input_pins().size(); icol++)
                {
                    u8 inputBit = u8((irow >> icol) & 1);
                    QTableWidgetItem* item = new QTableWidgetItem(inputBit ? "H" : "L");
                    mTableWidget->setItem(irow, icol, item);
                }

                //fill the output columns
                for (uint icol = gate->get_input_pins().size(); icol < gate->get_output_pins().size()+gate->get_input_pins().size(); icol++)
                {
                    BooleanFunction::Value val = truthTable[irow];
                    if (val == BooleanFunction::Value::ZERO)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("L");
                        mTableWidget->setItem(irow, icol, item);
                    }
                    else if (val == BooleanFunction::Value::ONE)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("H");
                        mTableWidget->setItem(irow, icol, item);
                    }
                    else if (val == BooleanFunction::Value::Z)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("Z");
                        mTableWidget->setItem(irow, icol, item);
                    }
                    else
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("X");
                        mTableWidget->setItem(irow, icol, item);
                        undefinedResult = true;
                    }
                }
            }
            if (undefinedResult)
            {
                mDisclaimer->setText("Truth table calculation\nfor gate type <" + QString::fromStdString(gate->get_name()) + ">\nnot implemented so far");
                mDisclaimer->show();
                mTableWidget->hide();
            }
            else
            {
                mDisclaimer->hide();
                mTableWidget->show();
            }
        }
        else
        {
            mDisclaimer->setText("No gate type selected");
            mDisclaimer->show();
            mTableWidget->hide();
        }
    }

    int GateLibraryTabTruthTable::getRowNumber(GateType* gate)
    {
        return pow(2, gate->get_input_pins().size()); //iterate from 0..0 to 2^n
    }

    int GateLibraryTabTruthTable::getColumnNumber(GateType* gate)
    {
        if(gate)
        {
            return gate->get_input_pins().size()+gate->get_output_pins().size();
        }
        return 0;
    }
}
