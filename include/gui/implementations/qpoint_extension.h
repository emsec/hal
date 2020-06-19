#pragma once

#include <QPoint>
static bool operator<(const QPoint& p1, const QPoint& p2)
{
    if (p1.x() < p2.x())
        return true;
    else if (p2.x() < p1.x())
        return false;
    else if (p1.y() < p2.y())
        return true;
    else
        return false;
}
