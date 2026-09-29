#include "gui/graph_widget/layouters/coordinate_from_data.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include <QMap>
#include "hal_core/utilities/log.h"

namespace hal {

    CoordinateFromData::CoordinateFromData(int x_, int y_)
        : QPoint(x_,y_)
    {;}

    namespace
    {
        /**
         * The data map of a container plus its typed values, so that a coordinate lives in either place.
         */
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> dataWithParameters(const DataContainer* c)
        {
            auto data = c->get_data_map();
            for (const auto& [key, entry] : c->get_parameters())
            {
                data[{enum_to_string(key.first), key.second}] = {enum_to_string(entry.first.get_type()), entry.second};
            }
            return data;
        }
    }    // namespace

    CoordinateFromData CoordinateFromData::fromNode(const Node& nd)
    {
        Module* m;
        Gate* g;
        switch (nd.type()) {
        case Node::Module:
            m = gNetlist->get_module_by_id(nd.id());
            Q_ASSERT(m);
            return fromData(dataWithParameters(m));
        case Node::Gate:
            g = gNetlist->get_gate_by_id(nd.id());
            Q_ASSERT(g);
            if (g->has_location())
            {
                int lx = (int) floor(g->get_location_x()+0.5);
                int ly = (int) floor(g->get_location_y()+0.5);
                return CoordinateFromData(lx,ly);
            }
            return fromData(dataWithParameters(g));
        default:
            break;
        }
        return CoordinateFromData();
    }

    CoordinateFromData CoordinateFromData::fromData(const std::map<std::tuple<std::string, std::string>,
                                                    std::tuple<std::string, std::string> >& dc)
    {
        CoordinateFromData retval;

        for(const auto& [key, value] : dc)
        {
            QString keyTxt      = QString::fromStdString(std::get<1>(key));
            QString valueTxt    = QString::fromStdString(std::get<1>(value));
            if (keyTxt.isEmpty()) continue;
            QChar xy = keyTxt.at(0);
            keyTxt.remove(0,1);
            if (keyTxt.toUpper() != "_COORDINATE") continue;
            bool ok = false;
            int val = valueTxt.toInt(&ok);
            if (!ok) continue;
            if (xy.toUpper().unicode() == 'X')
                retval.setX(val);
            else if (xy.toUpper().unicode() == 'Y')
                retval.setY(val);
        }

        return retval;
    }

    CoordinateFromDataMap::CoordinateFromDataMap(const QSet<u32>& modules, const QSet<u32>& gates)
        : mUndefCount(0), mDoubleCount(0)
    {
        QHash<QPoint,int> posCount;
        for (u32 mid : modules)
        {
            hal::Node nd(mid, hal::Node::Module);
            CoordinateFromData cfd = CoordinateFromData::fromNode(nd);
            if (cfd.isUndefined())
                mUndefCount++;
            else
            {
                insertNode(nd, cfd);
                mPlacedModules.insert(mid);
            }
        }
        for (u32 gid : gates)
        {
            hal::Node nd(gid, hal::Node::Gate);
            CoordinateFromData cfd = CoordinateFromData::fromNode(nd);
            if (cfd.isUndefined())
                mUndefCount++;
            else
            {
                insertNode(nd, cfd);
                mPlacedGates.insert(gid);
            }
        }
    }

    void CoordinateFromDataMap::insertNode(const hal::Node& nd, const CoordinateFromData& cfd)
    {
        insert(nd,cfd);
        int n = mPositionHash[cfd]++;

        if (n) ++ mDoubleCount;
    }

    void CoordinateFromDataMap::clear()
    {
        mUndefCount  = 0;
        mPlacedCount = 0;
        mDoubleCount = 0;
        mPositionHash.clear();
        mPlacedGates.clear();
        mPlacedModules.clear();
    }

    bool CoordinateFromDataMap::isPlacementComplete() const
    {
        return mUndefCount == 0;
    }

    void CoordinateFromDataMap::simplify()
    {
        QMap<int,int> xGrid;
        QMap<int,int> yGrid;
        for (const CoordinateFromData& cfd : values())
        {
            xGrid[cfd.x()] = 0;
            yGrid[cfd.y()] = 0;
        }

        int x = 0;
        for (auto itx = xGrid.begin(); itx != xGrid.end(); ++itx)
            *itx = x++;

        int y = 0;
        for (auto ity = yGrid.begin(); ity != yGrid.end(); ++ity)
            *ity = y++;

        for (auto it = begin(); it != end(); ++it)
        {
            int px = xGrid.value(it.value().x());
            int py = yGrid.value(it.value().y());
            *it = CoordinateFromData(px,py);
        }
    }

    bool CoordinateFromDataMap::good() const
    {
        if (mDoubleCount!=0)
            log_info("gui", "multiple assignment to same gate coordinate(s)");

        return (!mPlacedGates.isEmpty() || !mPlacedModules.isEmpty())
                && mDoubleCount == 0;
    }
}
