
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_general.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ff.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "gui/gui_globals.h"
#include <QFrame>
#include <QScrollArea>

namespace hal
{
    //--------------------- general frame ---------------------------
    GatelibraryFrameGeneral::GatelibraryFrameGeneral(QWidget* parent)
        : GatelibraryComponentFrame("General", parent)
    {
        mNameLabel       = new GateLibraryLabel(true, " - ", this);
        mIdLabel         = new GateLibraryLabel(true, " - ", this);
        mPropertiesLabel = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "Name:",       this), mNameLabel);
        mLayout->addRow(new GateLibraryLabel(false, "ID:",         this), mIdLabel);
        mLayout->addRow(new GateLibraryLabel(false, "Properties:", this), mPropertiesLabel);
    }

    void GatelibraryFrameGeneral::update(GateType *gt)
    {
        if(!gt)
        {
            mNameLabel->setText("-");
            mIdLabel->setText("-");
            mPropertiesLabel->setText("-");
            return;
        }

        mNameLabel->setText(QString::fromStdString(gt->get_name()));
        mIdLabel->setText(QString::number(gt->get_id()));

        // add properties
        QString properties = "";

        bool first = true;
        for (GateTypeProperty p : gt->get_property_list()) {
            //add \n before each property except the first
            if(first)
                first = false;
            else
                properties.append("\n");

            properties.append(QString::fromStdString(enum_to_string(p)));
        }
        mPropertiesLabel->setText(properties);
    }

    //--------------------- boolean function frame ------------------
    GatelibraryFrameBoolean::GatelibraryFrameBoolean(QWidget* parent)
        : GatelibraryComponentFrame("Boolean Functions", parent)
    {;}

    void GatelibraryFrameBoolean::update(GateType *gt)
    {
        // Clear existing labels
        QLayoutItem* item;
        while ((item = mLayout->takeAt(0)) != nullptr)  {
            delete item->widget();
            delete item;
        }
        mLayout->insertRow(0, new GateLibraryLabel(false, "Boolean Functions", this));

        //add label and corresponding boolean function to the layout
        auto boolFunctions = gt->get_boolean_functions();
        auto list = QList<QPair<QString, BooleanFunction>>();

        // add boolean functions for each output
        for(std::pair<const std::basic_string<char>, BooleanFunction> bf : boolFunctions){
            GateLibraryLabel* label = new GateLibraryLabel(true, QString::fromStdString(bf.second.to_string()), this);
            mLayout->insertRow(1, new GateLibraryLabel(false, QString::fromStdString(bf.first), this), label);
        }

        if (boolFunctions.empty())
            hide();
        else
            show();
    }

    //--------------------- tab widget with all component frames ----
    GateLibraryTabGeneral::GateLibraryTabGeneral(QWidget* parent) : GateLibraryTabInterface(parent)
    {
        QVBoxLayout* topLayout = new QVBoxLayout(this);
        QScrollArea* scroll = new QScrollArea(this); // add scrollbar if needed
        QVBoxLayout* layout = new QVBoxLayout(scroll);

        mGeneralFrame = new GatelibraryFrameGeneral(this);
        layout->addWidget(mGeneralFrame);

        mFlipflopFrame = new GatelibraryFrameFF(this);
        layout->addWidget(mFlipflopFrame);
        mFlipflopFrame->hide();

        mBooleanFrame = new GatelibraryFrameBoolean(this);
        layout->addWidget(mBooleanFrame);
        mBooleanFrame->hide();

        topLayout->addWidget(scroll);
    }

    void GateLibraryTabGeneral::update(GateType* gt)
    {
        mGeneralFrame->update(gt);
        if(!gt){
            //TODO make default look
            mFlipflopFrame->hide();
            mBooleanFrame->hide();
            return;
        }

        mFlipflopFrame->update(gt);
        mBooleanFrame->update(gt);
    }
}
