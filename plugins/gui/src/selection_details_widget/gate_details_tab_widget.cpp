#include "gui/selection_details_widget/gate_details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"
#include "gui/new_selection_details_widget/new_gate_details_widget/gate_info_table.h"
#include "gui/new_selection_details_widget/new_gate_details_widget/gate_pin_tree.h"

#include "hal_core/netlist/gate.h"

namespace hal
{
    GateDetailsTabWidget::GateDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(":/icons/sel_gate");

        //general tab
        mGateInfoTable = new GateInfoTable(this);
        mGateInformationFrame = new DetailsFrameWidget(mGateInfoTable, "Gate Information", this);
        mGroupingsFrame = new DetailsFrameWidget(new QWidget(this), "Groupings", this); //replace QWidget

        QList<DetailsFrameWidget*> framesGeneralTab({mGateInformationFrame, mGroupingsFrame});
        addTab("General", framesGeneralTab);

        //pins tab
        mPinsTree = new GatePinTree(this);
        mPinsFrame = new DetailsFrameWidget(mPinsTree, "Pins", this);
        connect(mPinsTree, &GatePinTree::updateText, mPinsFrame, &DetailsFrameWidget::setText);

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
    }
 
    void GateDetailsTabWidget::setGate(Gate* gate)
    {
        //pass gate or other stuff to widgets
        mGateInfoTable->setGate(gate);
        mPinsTree->setGate(gate);

        hideOrShorMultiTab(gate);
    }

    void GateDetailsTabWidget::hideOrShorMultiTab(Gate* gate)
    {
        bool mustHide = false;

        GateType* type = gate->get_type();

        if(type)
        {
            std::set<hal::GateTypeProperty> gateTypeProperties = type->get_properties();
            std::set<hal::GateTypeProperty> relevantProperties {GateTypeProperty::lut, GateTypeProperty::latch, GateTypeProperty::ff};

            auto releventFind = find_first_of(begin(gateTypeProperties), end(gateTypeProperties), begin(relevantProperties), end(relevantProperties));

            if(releventFind != end(gateTypeProperties))
                showMultiTab(*releventFind);
            else
                mustHide = true;
        }
        else
        {
            mustHide = true;
        }

        if(mustHide)
            hideMultiTab();
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
