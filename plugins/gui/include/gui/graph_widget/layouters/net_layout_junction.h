#pragma once

#include <QPoint>
#include <QPointF>

#include <QList>
#include <QHash>
#include <QColor>
#include <QRect>
#include "gui/graph_widget/layouters/net_layout_point.h"

// #define JUNCTION_DEBUG = 1

class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QGraphicsScene;

const int DELTA = 20;
const int FIRST = 400;
const int GAP   = 200;

namespace hal {

    /**
     * Condensed lane index comprises information whether road is horizontal or vertical
     *
     * @ingroup graph-layouter
     */
    class LaneIndex
    {
        friend uint qHash(const LaneIndex& ri);
        int mIndex;
    public:
        enum HVIndex { Horizontal, Vertical };

        /**
         * Constructor
         * @param ihv Horizontal or Vertical
         * @param ilane Lane index
         */
        LaneIndex(HVIndex ihv, int ilane) : mIndex(ihv | (ilane << 1)) {;}

        /**
         * Cast to integer, mainly used as index
         */
        operator int () const { return mIndex; }

        /**
         * Increment to next parallel lane
         * @return The incremented index
         */
        LaneIndex& operator++() { mIndex += 2; return *this; }

        /**
         * Decrement to previous parallel lane
         * @return  The decremented index
         */
        LaneIndex& operator--() { mIndex -= 2; return *this; }

        /**
         * Getter for road direction
         * @return Horizontal or Vertical
         */
        HVIndex hvIndex() const { return (HVIndex) (mIndex & 1); }

        /**
         * Getter for lane index
         * @return The index (might be negative)
         */
        int laneIndex() const { return (mIndex >> 1); }

        /**
         * Convenience method to check whether road direction is horizontal
         * @return true if horizontal, false otherwise
         */
        bool isHorizontal() const { return hvIndex() == Horizontal; }

        /**
         * Convenience method to check whether road direction is vertical
         * @return true if vertical, false otherwise
         */
        bool isVertical() const { return hvIndex() == Vertical; }

        /**
         * Construct horizontal lane
         * @param ilane - lane index
         */
        static LaneIndex horizontal(int ilane) { return LaneIndex(Horizontal,ilane); }

        /**
         * Construct vertical lane
         * @param ilane - lane index
         */
        static LaneIndex vertical(int ilane) { return LaneIndex(Vertical,ilane); }
    };

    class NetLayoutJunctionWire;

    /**
     * Range including first and last grid point (potentially) on lane used by net id.
     * External nets leading to junction start at MinInf or MaxInf.
     *
     * @ingroup graph-layouter
     */
    class NetLayoutJunctionRange
    {
        u32 mNetId;
        int mFirst;
        int mLast;
    public:
        static const int MinInf = -32767;
        static const int MaxInf =  32767;

        NetLayoutJunctionRange(u32 netId_, int first, int last);

        /**
         * Check whether range instance would overlap with instance from other net
         * @param other Possibly conflicting range
         * @return true if nets are different and share at least a common point, false otherwise
         */
        bool conflict(const NetLayoutJunctionRange& other) const;

        /**
         * Checks whether position is within range including first and last grid point
         * @param pos Relevant coordinate from grid point to check
         * @return true if in range according to condition above, false otherwise
         */
        bool contains(int pos) const { return mFirst <= pos && pos <= mLast; }

        /**
         * Checks whether position is within range excluding first and last grid point
         * @param pos Relevant coordinate from grid point to check
         * @return true if in range according to condition above, false otherwise
         */
        bool innerPos(int pos) const { return mFirst < pos && pos < mLast; }

        /**
         * Checks whether ranges of same net can be joined to bigger range
         * @param other Range which might be joined
         * @return true if range can be joined, false otherwise
         */
        bool canJoin(const NetLayoutJunctionRange& other) const;

        /**
         * Checks whether range is entry from external net
         * @param iTestMax Test for MinInf if 0, MaxInf otherwise
         * @return true if range is entry from external net
         */
        bool isEntry(int iTestMax) const;

        /**
         * Checks whether two ranges are equal
         */
        bool operator==(const NetLayoutJunctionRange& other) const;

        /**
         * Checks whether range from wire is equal with this range
         */
        bool operator==(const NetLayoutJunctionWire& wire) const;

        /**
         * Getter for net ID
         * @return the net ID
         */
        u32 netId() const { return mNetId; }

        /**
         * Getter for first grid point position in range
         * @return the first grid point position in range
         */
        int first() const { return mFirst; }

        /**
         * Setter for first grid point position in range
         * @param fst the new first grid point position in range
         */
        void setFirst(int fst) { mFirst = fst; }

        /**
         * Getter for last grid point position in range
         * @return the last grid point position in range
         */
        int last() const { return mLast; }

        /**
         * Setter for last grid point position in range
         * @param lst the new last grid point position in range
         */
        void setLast(int lst)  { mLast  = lst; }

        /**
         * Getter for the difference between first and last grid point of range
         * @return the differnce between first and last
         */
        int length() const { return mLast-mFirst; }

        /**
         * Getter for the first or last grid position in range
         * @param iGetLast get first position if 0, last positon otherwise
         * @return the first or last grid position in range
         */
        int endPosition(int iGetLast) const;

        /**
         * Expand the range by joining other range
         * @param other The other range
         */
        void expand(const NetLayoutJunctionRange& other);

        /**
         * Return range parameter as human readable string
         * @return The string
         */
        QString toString() const { return QString("<%1|%2..%3>").arg(mNetId).arg(mFirst).arg(mLast); }

        /**
         * Generate an entry range
         * @param dir Direction (Left, Right, Up, Down)
         * @param ilane Lane index
         * @param netId Net ID
         * @return The range
         */
        static NetLayoutJunctionRange entryRange(NetLayoutDirection dir, int ilane, u32 netId);
    };

    /**
     * Single straight wire located in a range of a lane
     *
     * @ingroup graph-layouter
     */
    class NetLayoutJunctionWire
    {
    public:
        /**
         * Lane index
         */
        LaneIndex mIndex;

        /**
         * Range including net id
         */
        NetLayoutJunctionRange mRange;

        /**
         * constructor
         */
        NetLayoutJunctionWire(const NetLayoutJunctionRange& rng, const LaneIndex& li)
            : mIndex(li),  mRange(rng) {;}

        /**
         * Checks whether wire is entry from external net
         * @return true if wire is entry from external net, false otherwise
         */
        bool isEntry() const;
    };

    /**
     * List of nets entering the junction sorted by entry direction
     */
    class NetLayoutJunctionEntries
    {
    public:
        /**
         * Entries sorted by direction (Left, Right, Up, Down)
         */
        QList<u32> mEntries[4];

        /**
         * Get net ID from junction entry list / array
         * @param dir Direction (Left, Right, Up, Down)
         * @param ilane Lane index
         * @return Net ID
         */
        u32 id(NetLayoutDirection dir, int ilane) const { return mEntries[dir.index()].at(ilane); }

        /**
         * String output for debugging purpose
         */
        QString toString() const;

        /**
         * Dump junction entries to file to debug junction routing
         * @param pnt Net grid point
         */
        void dumpToFile(const QPoint& pnt) const;

        static QString gridPointName(const QPoint& p);
    };

    /**
     * Connection more than one gate pin with the same wire
     */
    class NetLayoutJunctionMultiPin
    {
    public:
        /**
         * The lane connecting the gate which was routed first
         */
        int mLane;

        /**
         * Other lanes (except mLane) connecting the net with the gate
         */
        QList<int> mConnector;

        /**
         * Constructor
         */
        NetLayoutJunctionMultiPin() : mLane(-1) {}
    };


    /**
     * @brief Single net to be routed through the junction
     *
     * At this point there can only be one entry point for each given direction.
     * @ingroup graph-layouter
     */
    class NetLayoutJunctionNet
    {
        u32 mPattern;       // binary pattern
        int mLaneIndex[4];  // index: Left = 0, Right = 1, Up = 2, Down = 3    value: -1 = unused
        int mEntries;
        bool mPlaced;
    public:
        QList<NetLayoutJunctionWire> mWires;
        QList<QPoint> mKnots;

        NetLayoutJunctionNet();
        void addEntry(NetLayoutDirection dir, int laneInx);
        QString toString() const;

        /**
         * Check whether search pattern is included in binary pattern
         * @param searchPattern Combination of Left (bit 0), Right (bit 1), Up (bit 2), Down (bit 3)
         * @return true if search pattern is included in binary pattern, false otherwise
         */
        bool hasPattern(u32 searchPattern) const;
        int laneIndex(NetLayoutDirection dir) const { return mLaneIndex[dir.index()]; }
        int numberEntries() const { return mEntries; }
        u32 pattern() const { return mPattern; }
        void setPattern(u32 pat) { mPattern = pat; }
        bool isPlaced() const { return mPlaced; }
        QList<NetLayoutJunctionWire> wireAtPos(int pos, LaneIndex::HVIndex hvi);
        void setPlaced() { mPlaced = true; }        
        void addWire(const NetLayoutJunctionWire& wire) { mWires.append(wire); }
        void replaceWire(const NetLayoutJunctionRange& rng, const NetLayoutJunctionWire& wire);
        QList<QPoint> junctionKnots() const { return mKnots; }
    };

    /**
     * @ingroup graph-layouter
     */
     class NetLayoutJunctionOccupied : public QList<NetLayoutJunctionRange>
    {
    public:
        bool conflict(const NetLayoutJunctionRange& test) const;
        bool canJoin(u32 netId, int pos) const;
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutJunctionOccupiedHash : public QHash<LaneIndex,NetLayoutJunctionOccupied>
    {
    public:
        enum AddType { None, Added, Merged };

        /**
         * Adds new entry for range or merges it
         * @param ri Road index where to add
         * @param rng Range to add, will be modified on merge
         * @return see AddType above.
         */
        AddType addOrMerge(const LaneIndex &ri, NetLayoutJunctionRange& rng);

#ifdef JUNCTION_DEBUG
        QList<QPair<u32,NetLayoutJunctionWire>> mHistory;
#endif
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutJunction
    {
    public:
        NetLayoutJunction(const NetLayoutJunctionEntries& entries);
        ~NetLayoutJunction() {;}
        QRect rect() const { return mRect; }
        NetLayoutJunctionNet netById(u32 id) const { return mNets.value(id); }

        enum ErrorType {StraightRouteError = -3, TRouteError = -2, CornerRouteError = -1, Ok = 0 };
        ErrorType lastError() const { return mError; }

#ifdef JUNCTION_DEBUG
        void toScene(QGraphicsScene* scene) const;
        void toSceneStep(QGraphicsScene* scene, int istep);
        int numberSteps() const { return mOccupied.mHistory.size(); }
#endif
    private:
        void routeT();
        void routeAllStraight(NetLayoutDirection dirFrom, NetLayoutDirection dirTo);
        void routeAllCorner(NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic);
        void routeSingleStraight(u32 netId, int iMain, int iroadIn, int iroadOut);
        void routeSingleSwap(u32 netId, int iMain, int iroadIn, int iroadOut);
        void routeSingleCorner(u32 netId, NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic);

        /**
         * Connect nets with multiple pins at same node
         * @param leftOrRight whether Node is to the left or right of node
         */
        void routeAllMultiPin(NetLayoutDirection leftOrRight);

        /**
         * Connect a single node where
         * @param netId
         * @param leftOrRight
         * @param nmpin
         */
        void routeSingleMultiPin(u32 netId, NetLayoutDirection leftOrRight, const NetLayoutJunctionMultiPin &nmpin);
        void findJunctions();
        void calculateRect();
        bool conflict(const LaneIndex& ri, const NetLayoutJunctionRange& testRng) const;
        void place(const LaneIndex& ri, const NetLayoutJunctionRange& range);
        bool canJoin(const LaneIndex& ri, u32 netId, int pos) const;

        QRect mRect;
        NetLayoutJunctionEntries mEntries;
        QHash<u32,NetLayoutJunctionNet> mNets;
        NetLayoutJunctionOccupiedHash mOccupied;
        int maxRoad[2];
        ErrorType mError;
    };

    class NetLayoutJunctionHash : public QHash<NetLayoutPoint,NetLayoutJunction*>
    {
    public:
        NetLayoutJunctionHash()  = default;
        ~NetLayoutJunctionHash() = default;
        void clearAll();
    };

    uint qHash(const LaneIndex& ri);
}

#ifdef JUNCTION_DEBUG
    void setColorMap(const hal::NetLayoutJunctionEntries& entries);
    QColor colorFromId(u32 netId);
#endif

