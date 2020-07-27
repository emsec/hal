#pragma once

#include "def.h"
#include <QWidget>
#include <QFont>

namespace hal
{
    class DetailsWidget : public QWidget
    {
        Q_OBJECT
    public:
        enum detailsType_t {ModuleDetails, GateDetails, NetDetails};

        explicit DetailsWidget(detailsType_t tp, QWidget *parent = nullptr);

        QFont keyFont() const;
        u32   currentId() const;
        QString detailsTypeName() const;
    protected:
        detailsType_t m_detailsType;
        u32           m_currentId;
        QFont         m_keyFont;
        bool          m_hideEmptySections;
    };
}
