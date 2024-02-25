
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_boolean_function.h"

#include "gui/gui_globals.h"


namespace hal
{

    GateLibraryTabBooleanFunction::GateLibraryTabBooleanFunction(QWidget* parent) : GateLibraryTabInterface(parent)
    {
        mLayout = new QGridLayout(parent);
        mTableWidget = new QTableWidget();

        mLayout->addWidget(mTableWidget);

        //mHeaderView = new QHeaderView(Qt::Horizontal);
        //mTableWidget->setHorizontalHeader(mHeaderView);


        setLayout(mLayout);

    }

    void GateLibraryTabBooleanFunction::update(GateType* gate)
    {
        if(gate)
        {

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
            auto truthTable = boolFunc.compute_truth_table(inputs, false).get().at(0);

            for (int column = 0; column < mTableWidget->columnCount()-1; column++) {

                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(inputs[column]));
                mTableWidget->setItem(0, column, item);
            }

             mTableWidget->setItem(0, mTableWidget->columnCount()-1, new QTableWidgetItem(QString::fromStdString(gate->get_output_pin_names()[0])));

            for (int truthTableIdx = 0; truthTableIdx < truthTable.size(); truthTableIdx++)
            {
                //iterate from 0..0 to 1..1
                for (int i = 0; i < gate->get_input_pins().size(); i++)
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
                //get output
                BooleanFunction::Value val = truthTable[truthTableIdx];
                if (val == BooleanFunction::Value::ZERO)
                {
                    QTableWidgetItem* item = new QTableWidgetItem("L");
                    mTableWidget->setItem(truthTableIdx+1, mTableWidget->columnCount()-1, item);
                }
                else if (val == BooleanFunction::Value::ONE)
                {
                    QTableWidgetItem* item = new QTableWidgetItem("H");
                    mTableWidget->setItem(truthTableIdx+1, mTableWidget->columnCount()-1, item);
                }
                else if (val == BooleanFunction::Value::Z)
                {
                    QTableWidgetItem* item = new QTableWidgetItem("Z");
                    mTableWidget->setItem(truthTableIdx+1, mTableWidget->columnCount()-1, item);
                }
                else
                {
                    QTableWidgetItem* item = new QTableWidgetItem("X");
                    mTableWidget->setItem(truthTableIdx+1, mTableWidget->columnCount()-1, item);
                }
            }
        }
    }

    int GateLibraryTabBooleanFunction::getRowNumber(GateType* gate)
    {
        return pow(2, getColumnNumber(gate)-1)+1; //iterate from 0..0 to 2^n
    }

    int GateLibraryTabBooleanFunction::getColumnNumber(GateType* gate)
    {
        if(gate)
        {
            return gate->get_input_pins().size()+1;
        }
        return 0;
    }

}
