#include "gui/graph_widget/layouters/position_generator.h"

namespace hal
{
   PositionGenerator::PositionGenerator()
       : n(0), k(0)
   {;}

   QPoint PositionGenerator::position() const
   {
       if (k < n)
           return QPoint(n,k); // right edge
       if (k < 2*n)
           return QPoint(k-n,n); // lower edge
       return QPoint(n,n);
   }

   QPoint PositionGenerator::next()
   {
       if (k < 2*n) ++k;
       else
       {
           k = 0;
           ++n;
       }
       return position();
   }
}
