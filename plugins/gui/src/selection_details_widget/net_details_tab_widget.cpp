#include "gui/selection_details_widget/net_details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetDetailsTabWidget::NetDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(":/icons/sel_net");

        //create all widgets and replace the qWidgets in the frames with them

        //general tab
        mNetInformationFrame = new DetailsFrameWidget(new QWidget(this), "Net Information", this); //replace QWidget
        mGroupingsFrame = new DetailsFrameWidget(new QWidget(this), "Groupings", this); //replace QWidget
        mModulesFrame = new DetailsFrameWidget(new QWidget(this), "Modules", this); //replace QWidget

        QList<DetailsFrameWidget*> framesGeneralTab({mNetInformationFrame, mGroupingsFrame, mModulesFrame});
        addTab("General", framesGeneralTab);

        //sources / destinations tab
        mSourcesFrame = new DetailsFrameWidget(new QWidget(this), "Sources", this); // replace QWidget
        mDestinationsFrame = new DetailsFrameWidget(new QWidget(this), "Destinations", this); // replace QWidget


        QList<DetailsFrameWidget*> framesSrcDstTab({mSourcesFrame, mDestinationsFrame});
        addTab("Src / Dst", framesSrcDstTab);

        //data tab
        mDataFrame = new DetailsFrameWidget(new QWidget(this), "Data", this); //replace QWidget

        QList<DetailsFrameWidget*> framesDataTab({mDataFrame});
        addTab("Data", framesDataTab);

        //connect widgets with frames for refreshing the headers when necessary
    }
 
    void NetDetailsTabWidget::setNet(Net* net)
    {
        //pass net to widgets
    }
}
