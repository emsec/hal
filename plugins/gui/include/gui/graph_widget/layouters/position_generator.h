#pragma once

#include <QPoint>

namespace hal
{
    class PositionGenerator
    {
        int n;  // size of square
        int k;  // position on square edge
    public:
        PositionGenerator();

        QPoint position() const;
        QPoint next();
    };
}
