#include "gui/graph_widget/layout_locker.h"
#include "gui/graph_widget/contexts/graph_context.h"

namespace hal {

    LayoutLockerManager* LayoutLockerManager::inst = nullptr;

    LayoutLockerManager::LayoutLockerManager()
        : mLockCount(0) {;}

    LayoutLockerManager* LayoutLockerManager::instance()
    {
        if (!inst) inst = new LayoutLockerManager;
        return inst;
    }

    void LayoutLockerManager::addLock()
    {
        ++mLockCount;
    }

    void LayoutLockerManager::removeLock()
    {
        --mLockCount;
        if (mLockCount <= 0 && !mWaitingRoom.isEmpty())
        {
            for (GraphContext* ctx : mWaitingRoom)
                ctx->startSceneUpdate();
            mWaitingRoom.clear();
        }
    }

    bool LayoutLockerManager::canUpdate(GraphContext* ctx)
    {
        if (mLockCount <= 0) return true;
        mWaitingRoom.insert(ctx);
        return false;
    }

    void LayoutLockerManager::removeWaitingContext(GraphContext* ctx)
    {
        auto it = mWaitingRoom.find(ctx);
        if (it != mWaitingRoom.end())
            mWaitingRoom.erase(it);
    }

    LayoutLocker::LayoutLocker()
    {
        LayoutLockerManager::instance()->addLock();
    }

    LayoutLocker::~LayoutLocker()
    {
        LayoutLockerManager::instance()->removeLock();
    }
}
