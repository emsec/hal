#include "gui/content_anchor/content_anchor.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/content_frame/content_frame.h"
#include "gui/content_widget/content_widget.h"
#include "gui/settings/settings_manager.h"

namespace hal {
    int ContentAnchor::count() const
    {
        return mDockBar->count();
    }

    void ContentAnchor::saveState() const
    {
        QList<const ContentWidget*> widgets;
        for (int i=0; i<mDockBar->count(); i++)
        {
            const ContentWidget* cw = mDockBar->widgetAt(i);
            widgets.append(cw);
        }
        SettingsManager::instance()->widgetsSaveGeometry(mAnchorPosition, widgets);
    }

    QString ContentLayout::positionToString(Position pos)
    {
        QMetaEnum me = QMetaEnum::fromType<Position>();
        return QString(me.key(pos));
    }

    ContentLayout::Position ContentLayout::positionFromString(const QString& s)
    {
        QMetaEnum me = QMetaEnum::fromType<Position>();
        for (int p = 0; p < 4; p++)
            if (s == me.key(p))
            {
                return static_cast<Position>(p);
            }
        return Position::Right;
    }

}
