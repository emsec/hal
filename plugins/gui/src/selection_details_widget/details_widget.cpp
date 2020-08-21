#include "gui_globals.h"
#include "selection_details_widget/details_widget.h"

namespace hal
{
    DetailsWidget::DetailsWidget(detailsType_t tp, QWidget *parent)
        : QWidget(parent), m_detailsType(tp), m_currentId(0)
    {
        m_keyFont = QFont("Iosevka");
        m_keyFont.setBold(true);
        m_keyFont.setPixelSize(13);

        m_hideEmptySections = g_settings_manager.get("selection_details/hide_empty_sections", false).toBool();
    }

    QFont DetailsWidget::keyFont() const
    {
        return m_keyFont;
    }

    QString DetailsWidget::detailsTypeName() const
    {
        // enum detailsType_t {ModuleDetails, GateDetails, NetDetails};
        const char* typeName[] = { "module", "gate", "net"};
        return QString(typeName[m_detailsType]);
    }

}
