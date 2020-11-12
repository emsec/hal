#include "gui/graph_widget/layouters/position_generator.h"

namespace hal
{
   PositionGenerator::PositionGenerator()
       : mN(0), mK(0)
   {;}

   QPoint PositionGenerator::position() const
   {
       if (mK < mN)
           return QPoint(mN,mK); // right edge
       if (mK < 2*mN)
           return QPoint(mK-mN,mN); // lower edge
       return QPoint(mN,mN);
   }

   QPoint PositionGenerator::next()
   {
       if (mK < 2*mN) ++mK;
       else
       {
           mK = 0;
           ++mN;
       }
       return position();
   }
}
