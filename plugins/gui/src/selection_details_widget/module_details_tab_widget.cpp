#include "gui/selection_details_widget/module_details_tab_widget.h"

#include "gui/selection_details_widget/details_frame_widget.h"
#include "gui/selection_details_widget/module_details_widget/module_info_table.h"
#include "gui/selection_details_widget/module_details_widget/module_ports_tree.h"
#include "gui/selection_details_widget/module_details_widget/module_elements_tree.h"
#include "gui/comment_system/widgets/comment_widget.h"

#include "hal_core/netlist/module.h"

namespace hal
{
    ModuleDetailsTabWidget::ModuleDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(SelectionDetailsIconProvider::ModuleIcon);

        //general tab
        mModuleInfoTable = new ModuleInfoTable(this);
        mModuleInformationFrame = new DetailsFrameWidget(mModuleInfoTable, "Module Information", this);

        mGroupingsOfItemTable = new GroupingsOfItemWidget;
        mGroupingsFrame = new DetailsFrameWidget(mGroupingsOfItemTable, "Groupings", this);
        connect(mGroupingsOfItemTable,  &GroupingsOfItemWidget::updateText, mGroupingsFrame, &DetailsFrameWidget::setText);

        QList<DetailsFrameWidget*> framesGeneralTab({mModuleInformationFrame, mGroupingsFrame});
        addTab("General", framesGeneralTab);

        //ports tab
        mPinsTree = new ModulePinsTree(this);
        mPinsFrame = new DetailsFrameWidget(mPinsTree, "Pins", this);
        connect(mPinsTree, &ModulePinsTree::updateText, mPinsFrame, &DetailsFrameWidget::setText);

        QList<DetailsFrameWidget*> framesPortTab({mPinsFrame});
        addTab("Pins", framesPortTab);

        //elements tab
        mElementsTree = new ModuleElementsTree(this);
        mElementsFrame = new DetailsFrameWidget(mElementsTree, "Elements", this);

        QList<DetailsFrameWidget*> framesElementsTab({mElementsFrame});
        addTab("Elements", framesElementsTab);

        //data tab
        mDataTable = new DataTableWidget(this);
        mDataFrame = new DetailsFrameWidget(mDataTable, "Data", this);

        QList<DetailsFrameWidget*> framesDataTab({mDataFrame});
        addTab("Data", framesDataTab);

        //comment tab
        mCommentWidget = new CommentWidget(this);
        QTabWidget::addTab(mCommentWidget, "Comments");

    }
 
    void ModuleDetailsTabWidget::setModule(Module* module)
    {
        if (module) setIcon(SelectionDetailsIconProvider::ModuleIcon, module->get_id());
        //pass module or other stuff to widgets
        mModuleInfoTable->setModule(module);
        mPinsTree->setModule(module);
        mElementsTree->setModule(module);
        mGroupingsOfItemTable->setModule(module);
        mDataTable->setModule(module);
        mCommentWidget->nodeChanged(Node(module->get_id(), Node::NodeType::Module));
    }

    void ModuleDetailsTabWidget::clear()
    {
        mPinsTree->removeContent();
        mElementsTree->removeContent();
    }
}
