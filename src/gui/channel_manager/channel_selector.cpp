#include "channel_manager/channel_selector.h"

#include "channel_manager/channel_model.h"

channel_selector::channel_selector(QWidget* parent) : QComboBox(parent)
{
    channel_model* model = channel_model::get_instance();
    this->setModel(model);
}
