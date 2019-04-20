#include "splitter/splitter.h"

splitter::splitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent)
{
    setHandleWidth(1);
}

bool splitter::unused()
{
    for (int i = 0; i < count(); i++)
    {
        if (static_cast<QWidget*>(widget(i))->isVisible())
            return false;
    }
    return true;
}
