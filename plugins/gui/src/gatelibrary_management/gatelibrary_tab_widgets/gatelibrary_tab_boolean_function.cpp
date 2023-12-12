
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_boolean_function.h"

#include "gui/gui_globals.h"


namespace hal
{

    GateLibraryTabBooleanFunction::GateLibraryTabBooleanFunction(QWidget* parent) : QWidget(parent)
    {
        mFormLayout = new QFormLayout(parent);

        mPlaceholderLabel1 = new QLabel("Func 1", parent);
        mPlaceholderLabel2 = new QLabel("Func 2", parent);
        mPlaceholderLabel3 = new QLabel("Func 3", parent);
        mPlaceholderLabel4 = new QLabel("Func 4", parent);
        mPlaceholderLabel5 = new QLabel("Func 5", parent);

        mPlaceholderPropertyLabel1 = new QLabel(" - ", parent);
        mPlaceholderPropertyLabel2 = new QLabel(" - ", parent);
        mPlaceholderPropertyLabel3 = new QLabel(" - ", parent);
        mPlaceholderPropertyLabel4 = new QLabel(" - ", parent);
        mPlaceholderPropertyLabel5 = new QLabel(" - ", parent);

        mFormLayout->addRow(mPlaceholderLabel1, mPlaceholderPropertyLabel1);
        mFormLayout->addRow(mPlaceholderLabel2, mPlaceholderPropertyLabel2);
        mFormLayout->addRow(mPlaceholderLabel3, mPlaceholderPropertyLabel3);
        mFormLayout->addRow(mPlaceholderLabel4, mPlaceholderPropertyLabel4);
        mFormLayout->addRow(mPlaceholderLabel5, mPlaceholderPropertyLabel5);

        setLayout(mFormLayout);

    }

    void GateLibraryTabBooleanFunction::update(GateType* gate)
    {

        if(!gate){
            //TODO make default look
            mPlaceholderPropertyLabel1->setText("-");
            mPlaceholderPropertyLabel2->setText("-");
            mPlaceholderPropertyLabel3->setText("-");
            mPlaceholderPropertyLabel4->setText("-");
            mPlaceholderPropertyLabel5->setText("-");

            return;
        }

        QString str = "";
        for(auto pair : gate->get_boolean_functions()){
            str.append(QString::fromStdString(pair.first) + "   " + QString::fromStdString(pair.second.to_string()) + "\n");
        }
        mPlaceholderPropertyLabel1->setText(str);


    }


}
