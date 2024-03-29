
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_general.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ff.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_state.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_lut.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_init.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_latch.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ram.h"
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ram_port.h"

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

        mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
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
    {
        mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    }

    void GatelibraryFrameBoolean::update(GateType *gt)
    {
        // Clear existing labels
        QLayoutItem* item;
        if (!mLayout->isEmpty())
        {
            while ((item = mLayout->takeAt(0)) != nullptr)  {
                delete item->widget();
                delete item;
            }
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

        //QScrollArea* scroll = new QScrollArea(this); // add scrollbar if needed
        //QVBoxLayout* layout = new QVBoxLayout(scroll);

        QScrollArea* scroll = new QScrollArea();
        QVBoxLayout* layout = new QVBoxLayout();
        QFrame* content = new QFrame();


        mGeneralFrame = new GatelibraryFrameGeneral(this);
        layout->addWidget(mGeneralFrame);

        mFlipflopFrame = new GatelibraryFrameFF(this);
        layout->addWidget(mFlipflopFrame);
        mFlipflopFrame->hide();

        mStateFrame = new GatelibraryFrameState(this);
        layout->addWidget(mStateFrame);
        mStateFrame->hide();

        mLutFrame = new GatelibraryFrameLut(this);
        layout->addWidget(mLutFrame);
        mLutFrame->hide();

        mInitFrame = new GatelibraryFrameInit(this);
        layout->addWidget(mInitFrame);
        mInitFrame->hide();

        mBooleanFrame = new GatelibraryFrameBoolean(this);
        layout->addWidget(mBooleanFrame);
        mBooleanFrame->hide();

        mLatchFrame = new GateLibraryFrameLatch(this);
        layout->addWidget(mLatchFrame);
        mLatchFrame->hide();

        mRAMFrame = new GateLibraryFrameRAM(this);
        layout->addWidget(mRAMFrame);
        mRAMFrame->hide();

        mRAMPortFrame = new GateLibraryFrameRAMPort(this);
        layout->addWidget(mRAMPortFrame);
        mRAMPortFrame->hide();

        scroll->setWidgetResizable(true);

        content->setLayout(layout);
        topLayout->addWidget(scroll);
        scroll->setWidget(content);


        //topLayout->addWidget(scroll);
    }

    void GateLibraryTabGeneral::update(GateType* gt)
    {
        mGeneralFrame->update(gt);
        if(!gt){
            //TODO make default look
            mFlipflopFrame->hide();
            mStateFrame->hide();
            mLutFrame->hide();
            mInitFrame->hide();
            mBooleanFrame->hide();
            mLatchFrame->hide();
            mRAMFrame->hide();
            mRAMPortFrame->hide();
            return;
        }

        mFlipflopFrame->update(gt);
        mStateFrame->update(gt);
        mLutFrame->update(gt);
        mInitFrame->update(gt);
        mBooleanFrame->update(gt);
        mLatchFrame->update(gt);
        mRAMFrame->update(gt);
        mRAMPortFrame->update(gt);
    }
}
