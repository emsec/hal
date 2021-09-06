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

        mGroupingsOfItemTable = new GroupingsOfItemWidget;
        mGroupingsFrame = new DetailsFrameWidget(mGroupingsOfItemTable, "Groupings", this); //replace QWidget
        connect(mGroupingsOfItemTable,  &GroupingsOfItemWidget::updateText, mGroupingsFrame, &DetailsFrameWidget::setText);

        QList<DetailsFrameWidget*> framesGeneralTab({mGateInformationFrame, mGroupingsFrame});
        addTab("General", framesGeneralTab);

        //pins tab
        mPinsTree = new GatePinTree(this);
        mPinsFrame = new DetailsFrameWidget(mPinsTree, "Pins", this);
        connect(mPinsTree, &GatePinTree::updateText, mPinsFrame, &DetailsFrameWidget::setText);

        QList<DetailsFrameWidget*> framesPinsTab({mPinsFrame});
        addTab("Pins", framesPinsTab);

        //(ff / latch / lut) tab - would love to use seperate tabs, but it's a hassle to hide multiple individual tabs witouth setTabVisible() from qt 5.15
        mFfFunctionTable = new BooleanFunctionTable(this);
        mLatchFunctionTable = new BooleanFunctionTable(this);
        mLutFunctionTable = new BooleanFunctionTable(this);
        mLutTable = new LUTTableWidget(this);

        mFfFrame = new DetailsFrameWidget(mFfFunctionTable, "FF Information", this);
        mLatchFrame = new DetailsFrameWidget(mLatchFunctionTable, "Latch Information", this);
        mLutFrame = new DetailsFrameWidget(mLutFunctionTable, "LUT Information", this); 
        mTruthTableFrame = new DetailsFrameWidget(mLutTable, "Truth Table", this); 

        QList<DetailsFrameWidget*> framesFfLatchLutTab({mFfFrame, mLatchFrame, mLutFrame, mTruthTableFrame});
        mMultiTabIndex = addTab("(FF / Latch / LUT)", framesFfLatchLutTab); //save index of multi tab -> needed for show / hide
        mMultiTabContent = widget(mMultiTabIndex); // save content of multi tab -> needed for show / hide

        //boolean functions tab
        mFullFunctionTable = new BooleanFunctionTable(this);
        mBooleanFunctionsFrame = new DetailsFrameWidget(mFullFunctionTable, "Boolean Functions", this); 

        QList<DetailsFrameWidget*> framesBooleanFunctionsTab({mBooleanFunctionsFrame});
        addTab("Boolean Functions", framesBooleanFunctionsTab);

        //data tab
        mDataTable = new DataTableWidget(this);
        mDataFrame = new DetailsFrameWidget(mDataTable, "Data", this);
        
        QList<DetailsFrameWidget*> framesDataTab({mDataFrame});
        addTab("Data", framesDataTab);
    }
 
    void GateDetailsTabWidget::setGate(Gate* gate)
    {
        //pass gate or other stuff to widgets
        mGateInfoTable->setGate(gate);
        mGroupingsOfItemTable->setGate(gate);
        mPinsTree->setGate(gate);
        mDataTable->setGate(gate);

        // Logic for LUT/FF/LATCH
        GateDetailsTabWidget::GateTypeCategory gateTypeCategory = getGateTypeCategory(gate);
        hideOrShorMultiTab(gateTypeCategory);
        setupBooleanFunctionTables(gate, gateTypeCategory);

    }

    void GateDetailsTabWidget::hideOrShorMultiTab(GateDetailsTabWidget::GateTypeCategory gateTypeCategory)
    {
        if(gateTypeCategory != GateDetailsTabWidget::GateTypeCategory::none)
        {
            showMultiTab(gateTypeCategory);
        }
        else
        {
            hideMultiTab();
        }   
    }

    void GateDetailsTabWidget::hideMultiTab()
    {
        if(mMultiTabVisible)
        {
            removeTab(mMultiTabIndex);
            mMultiTabVisible = false;
        }
    }

    void GateDetailsTabWidget::showMultiTab(GateDetailsTabWidget::GateTypeCategory gateTypeCategory)
    {
        QString label = "";

        switch(gateTypeCategory)
        {
            case GateDetailsTabWidget::GateTypeCategory::lut:
            {
                mLutFrame->setVisible(true);
                mTruthTableFrame->setVisible(true);
                mFfFrame->setVisible(false);
                mLatchFrame->setVisible(false);
                label = "LUT";
                break;
            }                    
            case GateDetailsTabWidget::GateTypeCategory::ff:
            {
                mLutFrame->setVisible(false);
                mTruthTableFrame->setVisible(false);
                mFfFrame->setVisible(true);
                mLatchFrame->setVisible(false);
                label = "FF";
                break;
            }
            case GateDetailsTabWidget::GateTypeCategory::latch:
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

    GateDetailsTabWidget::GateTypeCategory GateDetailsTabWidget::getGateTypeCategory(Gate* gate) const
    {
        if(gate == nullptr)
        {
            return GateDetailsTabWidget::GateTypeCategory::none;
        }

        GateType* type = gate->get_type();

        std::set<hal::GateTypeProperty> gateTypeProperties = type->get_properties();
        std::set<hal::GateTypeProperty> relevantProperties {GateTypeProperty::lut, GateTypeProperty::latch, GateTypeProperty::ff};

        auto relevantFind = find_first_of(begin(gateTypeProperties), end(gateTypeProperties), begin(relevantProperties), end(relevantProperties));

        if(relevantFind == end(gateTypeProperties))
        {
            return GateDetailsTabWidget::GateTypeCategory::none;
        }

        switch(*relevantFind)
        {
            case GateTypeProperty::lut:
            {
                return GateDetailsTabWidget::GateTypeCategory::lut;
                break;
            }                    
            case GateTypeProperty::ff:
            {
                return GateDetailsTabWidget::GateTypeCategory::ff;
                break;
            }
            case GateTypeProperty::latch:
            {
                return GateDetailsTabWidget::GateTypeCategory::latch;
                break;
            }
            default:
            {
                return GateDetailsTabWidget::GateTypeCategory::none; 
                break;
            } 
        }
        
    }

    void GateDetailsTabWidget::setupBooleanFunctionTables(Gate* gate, GateDetailsTabWidget::GateTypeCategory gateTypeCategory)
    {
        if(gate == nullptr){
            return;
        }

        static QSet<QString> ffBfNames = {
            "clear", "preset", // Both
            "clock", "clocked_on", "clocked_on_also", "next_state", "power_down_function" // FF names
        };

        static QSet<QString> latchBfNames = {
            "clear", "preset", // Both
            "enable", "data_in" // Latch names
        };

        static QSet<QString> noSpecialBfNames = {};

        QSet<QString>* specialBfNames = &noSpecialBfNames;
        if(gateTypeCategory == GateDetailsTabWidget::GateTypeCategory::ff)
        {
            specialBfNames = &ffBfNames;
        }
        else if(gateTypeCategory == GateDetailsTabWidget::GateTypeCategory::latch)
        {
            specialBfNames = &latchBfNames;
        }

        std::unordered_map<std::string, BooleanFunction> allBfs = gate->get_boolean_functions(false);

        QMap<QString, BooleanFunction> specialFunctions;
        QMap<QString, BooleanFunction> otherFunctions;

        for(auto& it : allBfs){
            QString bfName = QString::fromStdString(it.first);
            if(specialBfNames->contains(bfName))
            {
                // Function is a LUT/FF function
                specialFunctions.insert(bfName, it.second);
            }
            else
            {
                // Function is not a LUT/FF function
                otherFunctions.insert(bfName, it.second);
            }
        }

        QList<QSharedPointer<BooleanFunctionTableEntry>> specialFunctionList;
        QList<QSharedPointer<BooleanFunctionTableEntry>> otherFunctionList;
        QList<QSharedPointer<BooleanFunctionTableEntry>> setPresetBehavior;

        QMap<QString, BooleanFunction>::iterator i;

        for(i = specialFunctions.begin(); i != specialFunctions.end(); i++)
           specialFunctionList.append(QSharedPointer<BooleanFunctionTableEntry>(new BooleanFunctionEntry(gate->get_id(), i.key(), i.value())));

        for(i = otherFunctions.begin(); i != otherFunctions.end(); i++)
            otherFunctionList.append(QSharedPointer<BooleanFunctionTableEntry>(new BooleanFunctionEntry(gate->get_id(), i.key(), i.value())));

        setPresetBehavior.append(QSharedPointer<BooleanFunctionTableEntry>(new CPBehaviorEntry(gate->get_id(), gate->get_type()->get_clear_preset_behavior())));


        // Fill the category (LUT/FF/LATCH) widgets
        switch(gateTypeCategory)
        {
            case GateDetailsTabWidget::GateTypeCategory::lut:
            {
                mLutFunctionTable->setEntries(otherFunctionList);

                // Fill the LUL truth table
                std::unordered_set<std::basic_string<char>> lutPins = gate->get_type()->get_pins_of_type(PinType::lut);
                // The table is only updated if the gate has a LUT pin
                if(lutPins.size() > 0){
                    // All LUT pins have the same boolean function
                    std::basic_string<char> outPin = *lutPins.begin();
                    BooleanFunction lutFunction = gate->get_boolean_function(outPin);
                    mLutTable->setBooleanFunction(lutFunction, QString::fromStdString(outPin));
                }
                break;
            }                    
            case GateDetailsTabWidget::GateTypeCategory::ff:
            {
                mFfFunctionTable->setEntries(specialFunctionList + setPresetBehavior);
                break;
            }
            case GateDetailsTabWidget::GateTypeCategory::latch:
            {
                mLatchFunctionTable->setEntries(specialFunctionList + setPresetBehavior);
                break;
            }
            default: break;
        }
        mFullFunctionTable->setEntries(specialFunctionList + otherFunctionList);

    }

}
