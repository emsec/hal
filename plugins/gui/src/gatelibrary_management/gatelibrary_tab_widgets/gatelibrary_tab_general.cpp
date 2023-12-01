
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_general.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{

    GateLibraryTabGeneral::GateLibraryTabGeneral(QWidget* parent) : QWidget(parent)
    {
        mGridLayout = new QGridLayout(parent);

        mNameLabel = new QLabel("Name", parent);
        mIdLabel = new QLabel("ID", parent);
        mComponentLabel = new QLabel("Component", parent);
        mBooleanFunctionLabel = new QLabel("Boolean function", parent);
        mPinLabel = new QLabel("Pins", parent);

        mNamePropertyLabel = new QLabel(" - ", parent);
        mIdPropertyLabel = new QLabel(" - ", parent);
        mComponentPropertyLabel = new QLabel(" - ", parent);
        mBooleanFunctionPropertyLabel = new QLabel(" - ", parent);
        mPinPropertyLabel = new QLabel(" - ", parent);

        mGridLayout->addWidget(mIdLabel, 0, 0);
        mGridLayout->addWidget(mIdPropertyLabel, 0, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mNameLabel, 1, 0);
        mGridLayout->addWidget(mNamePropertyLabel, 1, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mComponentLabel, 2, 0);
        mGridLayout->addWidget(mComponentPropertyLabel, 2, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mBooleanFunctionLabel, 3, 0);
        mGridLayout->addWidget(mBooleanFunctionPropertyLabel, 3, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mPinLabel, 4, 0);
        mGridLayout->addWidget(mPinPropertyLabel, 4, 1, Qt::AlignLeft);

        setLayout(mGridLayout);

    }

    void GateLibraryTabGeneral::update(GateType* gate)
    {

        if(!gate){
            //TODO make default look
            mNamePropertyLabel->setText("-");
            mIdPropertyLabel->setText("-");
            mComponentPropertyLabel->setText("-");
            mBooleanFunctionPropertyLabel->setText("-");
            mPinPropertyLabel->setText("-");

            return;
        }

        mNamePropertyLabel->setText(QString::fromStdString(gate->get_name()));
        mIdPropertyLabel->setText(QString::number(gate->get_id()));
        //TODO add component
        mComponentPropertyLabel->setText("TODO");
        mBooleanFunctionPropertyLabel->setText(QString::fromStdString(gate->get_boolean_function().to_string()));

        QString in = "  ";
        QString out = "  ";

        for(auto const& pin : gate->get_input_pin_names()){
            in += QString::fromStdString(pin) + "\n  ";
        }

        for(auto const& pin : gate->get_output_pin_names()){
            out += QString::fromStdString(pin) + "\n  ";
        }

        mPinPropertyLabel->setText("In:\n" + in + "\nOut:\n" + out);





    }


}
