
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
                return;
            }

            mTableWidget->show();

            mTableWidget->setColumnCount(getColumnNumber(gate));
            mTableWidget->setRowCount(getRowNumber(gate));
            mTableWidget->verticalHeader()->hide();

            BooleanFunction boolFunc = gate->get_boolean_function();
            std::vector<std::string> inputs = gate->get_input_pin_names();
            std::vector<std::string> outputs = gate->get_output_pin_names();
            auto truthTable = boolFunc.compute_truth_table(inputs, false).get().at(0);

            for (uint column = 0; column < inputs.size(); column++) {

                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(inputs[column]));
                mTableWidget->setItem(0, column, item);
            }
            for (uint column = inputs.size(); column < outputs.size()+inputs.size(); column++) {

                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(outputs[column-inputs.size()]));
                mTableWidget->setItem(0, column, item);
            }

            for (uint truthTableIdx = 0; truthTableIdx < truthTable.size(); truthTableIdx++)
            {
                //iterate from 0..0 to 1..1
                for (uint i = 0; i < gate->get_input_pins().size(); i++)
                {
                    u32 shift   = gate->get_input_pins().size() - i - 1;
                    u8 inputBit = u8((truthTableIdx >> shift) & 1);
                    if(inputBit == 0)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("L");
                        mTableWidget->setItem(truthTableIdx+1, i, item);
                    }
                    else
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("H");
                        mTableWidget->setItem(truthTableIdx+1, i, item);
                    }


                }
                //fill the output columns
                for (int i = gate->get_input_pins().size(); i < gate->get_output_pins().size()+gate->get_input_pins().size(); i++)
                {
                    BooleanFunction::Value val = truthTable[truthTableIdx];
                    if (val == BooleanFunction::Value::ZERO)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("L");
                        mTableWidget->setItem(truthTableIdx+1, i, item);
                    }
                    else if (val == BooleanFunction::Value::ONE)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("H");
                        mTableWidget->setItem(truthTableIdx+1, i, item);
                    }
                    else if (val == BooleanFunction::Value::Z)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("Z");
                        mTableWidget->setItem(truthTableIdx+1, i, item);
                    }
                    else
                    {
                        QTableWidgetItem* item = new QTableWidgetItem("X");
                        mTableWidget->setItem(truthTableIdx+1, i, item);
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
        return pow(2, gate->get_input_pins().size())+1; //iterate from 0..0 to 2^n
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
