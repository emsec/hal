#pragma once
#include "user_action.h"

namespace hal
{
    class ActionOpenNetlistFile : public UserAction
    {
        QString mFilename;
    public:
        ActionOpenNetlistFile(const QString& filename_);
        void exec() override;
    };
}
