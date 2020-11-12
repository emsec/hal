#include "gui/gui_globals.h"
#include "gui/selection_details_widget/details_widget.h"

namespace hal
{
    DetailsWidget::DetailsWidget(DetailsType tp, QWidget *parent)
        : QWidget(parent), mDetailsType(tp), mCurrentId(0)
    {
        mKeyFont = QFont("Iosevka");
        mKeyFont.setBold(true);
        mKeyFont.setPixelSize(13);

        mHideEmptySections = gSettingsManager->get("selection_details/hide_empty_sections", false).toBool();
    }

    QFont DetailsWidget::keyFont() const
    {
        return mKeyFont;
    }

    QString DetailsWidget::detailsTypeName() const
    {
        // enum DetailsType {ModuleDetails, GateDetails, NetDetails};
        const char* typeName[] = { "module", "gate", "net"};
        return QString(typeName[mDetailsType]);
    }

}
