#pragma once
#include "user_action.h"

namespace hal
{
    class ActionCreateGrouping : public UserAction
    {
    public:
        ActionCreateGrouping(const QString& gname = QString());
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
    private:
        QString mGroupingName;
    };
}
