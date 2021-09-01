#include "gui/selection_details_widget/module_details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"
#include "gui/new_selection_details_widget/new_module_details_widget/module_info_table.h"
#include "gui/new_selection_details_widget/new_module_details_widget/module_ports_tree.h"
#include "gui/new_selection_details_widget/new_module_details_widget/module_elements_tree.h"

#include "hal_core/netlist/module.h"

namespace hal
{
    ModuleDetailsTabWidget::ModuleDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(":/icons/sel_module");

        //general tab
        mModuleInfoTable = new ModuleInfoTable(this);
        mModuleInformationFrame = new DetailsFrameWidget(mModuleInfoTable, "Module Information", this);
        mGroupingsFrame = new DetailsFrameWidget(new QWidget(this), "Groupings", this); //replace QWidget

        QList<DetailsFrameWidget*> framesGeneralTab({mModuleInformationFrame, mGroupingsFrame});
        addTab("General", framesGeneralTab);

        //ports tab
        mPortsTree = new ModulePortsTree(this);
        mPortsFrame = new DetailsFrameWidget(mPortsTree, "Ports", this);
        connect(mPortsTree, &ModulePortsTree::updateText, mPortsFrame, &DetailsFrameWidget::setText);

        QList<DetailsFrameWidget*> framesPortTab({mPortsFrame});
        addTab("Ports", framesPortTab);

        //elements tab
        mElementsTree = new ModuleElementsTree(this);
        mElementsFrame = new DetailsFrameWidget(mElementsTree, "Elements", this);

        QList<DetailsFrameWidget*> framesElementsTab({mElementsFrame});
        addTab("Elements", framesElementsTab);

        //data tab
        mDataFrame = new DetailsFrameWidget(new QWidget(this), "Data", this); //replace QWidget

        QList<DetailsFrameWidget*> framesDataTab({mDataFrame});
        addTab("Data", framesDataTab);
    }
 
    void ModuleDetailsTabWidget::setModule(Module* module)
    {
        //pass module or other stuff to widgets
        mModuleInfoTable->setModule(module);
        mPortsTree->setModule(module);
        mElementsTree->setModule(module);
    }
}
