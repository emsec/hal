#include "channel_manager/channel_selector.h"

#include "channel_manager/channel_model.h"
namespace hal
{
    ChannelSelector::ChannelSelector(QWidget* parent) : QComboBox(parent)
    {
        ChannelModel* model = ChannelModel::get_instance();
        this->setModel(model);
    }
}
