#include "gui/channel_manager/channel_selector.h"

#include "gui/channel_manager/channel_model.h"

#include <QKeyEvent>
#include <QLineEdit>

namespace hal
{
    ChannelSelector::ChannelSelector(QWidget* parent) : QComboBox(parent)
    {
        ChannelModel* model = ChannelModel::instance();
        this->setModel(model);
    }

    void ChannelSelector::keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
        {
            QString txt = lineEdit()->text();
            if (!txt.isEmpty())
            {
                ChannelModel* model = ChannelModel::instance();
                std::string cname = txt.toStdString();
                model->handleLogmanagerCallback(spdlog::level::level_enum::debug , cname, cname + " has manually been added to channellist");
                return;
            }
        }
        QComboBox::keyPressEvent(e);
    }

}
