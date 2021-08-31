#include "gui/selection_details_widget/gate_details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"
#include "hal_core/netlist/gate.h"

namespace hal
{
    GateDetailsTabWidget::GateDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(":/icons/sel_gate");

        //create all widgets and replace the qWidgets in the frames with them

        //general tab
        mGateInformationFrame = new DetailsFrameWidget(new QWidget(this), "Gate Information", this); //replace QWidget
        mGroupingsFrame = new DetailsFrameWidget(new QWidget(this), "Groupings", this); //replace QWidget

        QList<DetailsFrameWidget*> framesGeneralTab({mGateInformationFrame, mGroupingsFrame});
        addTab("General", framesGeneralTab);

        //pins tab
        mPinsFrame = new DetailsFrameWidget(new QWidget(this), "Pins", this); // replace QWidget

        QList<DetailsFrameWidget*> framesPinsTab({mPinsFrame});
        addTab("Pins", framesPinsTab);

        //(ff / latch / lut) tab - would love to use seperate tabs, but it's a hassle to hide multiple individual tabs witouth setTabVisible() from qt 5.15
        mFfFrame = new DetailsFrameWidget(new QWidget(this), "FF Information", this); //replace QWidget
        mLatchFrame = new DetailsFrameWidget(new QWidget(this), "Latch Information", this); //replace QWidget
        mLutFrame = new DetailsFrameWidget(new QWidget(this), "LUT Information", this); //replace QWidget
        mTruthTableFrame = new DetailsFrameWidget(new QWidget(this), "Truth Table", this); //replace QWidget

        QList<DetailsFrameWidget*> framesFfLatchLutTab({mFfFrame, mLatchFrame, mLutFrame, mTruthTableFrame});
        mMultiTabIndex = addTab("(FF / Latch / LUT)", framesFfLatchLutTab); //save index of multi tab -> needed for show / hide
        mMultiTabContent = widget(mMultiTabIndex); // save content of multi tab -> needed for show / hide

        //boolean functions tab
        mBooleanFunctionsFrame = new DetailsFrameWidget(new QWidget(this), "Boolean Functions", this); //replace QWidget

        QList<DetailsFrameWidget*> framesBooleanFunctionsTab({mBooleanFunctionsFrame});
        addTab("Boolean Functions", framesBooleanFunctionsTab);

        //data tab
        mDataFrame = new DetailsFrameWidget(new QWidget(this), "Data", this); //replace QWidget

        QList<DetailsFrameWidget*> framesDataTab({mDataFrame});
        addTab("Data", framesDataTab);

        //connect widgets with frames for refreshing the headers when necessary
    }
 
    void GateDetailsTabWidget::setGate(Gate* gate)
    {
        
        GateType* type = gate->get_type();

        if(type)
        {
            std::set<hal::GateTypeProperty> properties = type->get_properties();

            auto relevantProperty = std::find_if(std::begin(properties), std::end(properties), [](GateTypeProperty gtp)
            {
                return gtp == GateTypeProperty::lut || gtp == GateTypeProperty::latch || gtp == GateTypeProperty::ff;
            });

            if(relevantProperty != std::end(properties))
                showMultiTab(*relevantProperty);
            else
                hideMultiTab();
        }
        else
        {
            hideMultiTab();
        }

        //pass gate to widgets
    }

    void GateDetailsTabWidget::hideMultiTab()
    {
        if(mMultiTabVisible)
        {
            removeTab(mMultiTabIndex);
            mMultiTabVisible = false;
        }
    }

    void GateDetailsTabWidget::showMultiTab(GateTypeProperty gateTypeProperty)
    {
        QString label = "";

        switch(gateTypeProperty)
        {
            case GateTypeProperty::lut:
            {
                mLutFrame->setVisible(true);
                mTruthTableFrame->setVisible(true);
                mFfFrame->setVisible(false);
                mLatchFrame->setVisible(false);
                label = "LUT";
                break;
            }                    
            case GateTypeProperty::ff:
            {
                mLutFrame->setVisible(false);
                mTruthTableFrame->setVisible(false);
                mFfFrame->setVisible(true);
                mLatchFrame->setVisible(false);
                label = "FF";
                break;
            }
            case GateTypeProperty::latch:
            {
                mLutFrame->setVisible(false);
                mTruthTableFrame->setVisible(false);
                mFfFrame->setVisible(false);
                mLatchFrame->setVisible(true);
                label = "Latch";
                break;
            }
            default: break;
        }

        if(!mMultiTabVisible)
        {
            insertTab(mMultiTabIndex, mMultiTabContent, label);
            mMultiTabVisible = true;
        }
        else
        {
            setTabText(mMultiTabIndex, label);
        }
    }
}
