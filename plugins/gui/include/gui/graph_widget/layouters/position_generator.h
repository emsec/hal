#pragma once

#include <QPoint>

namespace hal
{
    /**
     * @ingroup graph-layouter
     */
    class PositionGenerator
    {
        int mN;  // size of square
        int mK;  // position on square edge
    public:
        PositionGenerator();

        QPoint position() const;
        QPoint next();
    };
}
