#include "gui/selection_details_widget/module_details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"
#include "hal_core/netlist/module.h"

namespace hal
{
    ModuleDetailsTabWidget::ModuleDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(":/icons/sel_module");

        //create all widgets and replace the qWidgets in the frames with them

        //general tab
        mModuleInformationFrame = new DetailsFrameWidget(new QWidget(this), "Module Information", this); //replace QWidget
        mGroupingsFrame = new DetailsFrameWidget(new QWidget(this), "Groupings", this); //replace QWidget

        QList<DetailsFrameWidget*> framesGeneralTab({mModuleInformationFrame, mGroupingsFrame});
        addTab("General", framesGeneralTab);

        //ports tab
        mPortsFrame = new DetailsFrameWidget(new QWidget(this), "Ports", this); // replace QWidget

        QList<DetailsFrameWidget*> framesPortTab({mPortsFrame});
        addTab("Ports", framesPortTab);

        //items tab
        mItemsFrame = new DetailsFrameWidget(new QWidget(this), "Ports", this); // replace QWidget

        QList<DetailsFrameWidget*> framesItemsTab({mItemsFrame});
        addTab("Items", framesItemsTab);

        //data tab
        mDataFrame = new DetailsFrameWidget(new QWidget(this), "Data", this); //replace QWidget

        QList<DetailsFrameWidget*> framesDataTab({mDataFrame});
        addTab("Data", framesDataTab);

        //connect widgets with frames for refreshing the headers when necessary
    }
 
    void ModuleDetailsTabWidget::setModule(Module* module)
    {
        //pass module to widgets
    }
}
