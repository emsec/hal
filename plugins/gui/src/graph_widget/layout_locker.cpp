#include "gui/graph_widget/layout_locker.h"
#include "gui/gui_globals.h"
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
            for (u32 ctxId : mWaitingRoom)
            {
                GraphContext* ctx = gGraphContextManager->getContextById(ctxId);
                if (ctx) ctx->startSceneUpdate();
            }
            mWaitingRoom.clear();
        }
    }

    bool LayoutLockerManager::canUpdate(GraphContext* ctx)
    {
        if (!ctx) return false;
        if (mLockCount <= 0) return true;
        mWaitingRoom.insert(ctx->id());
        return false;
    }

    void LayoutLockerManager::removeWaitingContext(GraphContext* ctx)
    {
        auto it = mWaitingRoom.find(ctx->id());
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
