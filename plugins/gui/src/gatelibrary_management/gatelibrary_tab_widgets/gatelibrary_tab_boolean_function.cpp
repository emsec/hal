
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_boolean_function.h"

#include "gui/gui_globals.h"


namespace hal
{

    GateLibraryTabBooleanFunction::GateLibraryTabBooleanFunction(QWidget* parent) : QWidget(parent)
    {
        mGridLayout = new QGridLayout(parent);

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

        mGridLayout->addWidget(mPlaceholderLabel2, 0, 0);
        mGridLayout->addWidget(mPlaceholderPropertyLabel2, 0, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mPlaceholderLabel1, 1, 0);
        mGridLayout->addWidget(mPlaceholderPropertyLabel1, 1, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mPlaceholderLabel3, 2, 0);
        mGridLayout->addWidget(mPlaceholderPropertyLabel3, 2, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mPlaceholderLabel4, 3, 0);
        mGridLayout->addWidget(mPlaceholderPropertyLabel4, 3, 1, Qt::AlignLeft);
        mGridLayout->addWidget(mPlaceholderLabel5, 4, 0);
        mGridLayout->addWidget(mPlaceholderPropertyLabel5, 4, 1, Qt::AlignLeft);

        setLayout(mGridLayout);

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
