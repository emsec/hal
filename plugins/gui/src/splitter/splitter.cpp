#include "gui/splitter/splitter.h"

namespace hal
{
    Splitter::Splitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent)
    {
        setHandleWidth(1);
    }

    bool Splitter::unused()
    {
        for (int i = 0; i < count(); i++)
        {
            if (static_cast<QWidget*>(widget(i))->isVisible())
                return false;
        }
        return true;
    }
}
