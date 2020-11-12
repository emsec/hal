#include "gui/main_window/about_dialog.h"
#include "hal_core/utilities/utils.h"

namespace hal
{
    AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), mPlaceholder("about gui"), mLicensesLabel("Open Source licenses:"), mTextedit()
    {
        setLayout(&mContentLayout);
        mPlaceholder.setText(QString("Compiled with Qt Version ") + QString(QT_VERSION_STR));
        mTextedit.setPlainText(QString::fromStdString(utils::get_open_source_licenses()));
        mTextedit.setReadOnly(true);
        mContentLayout.addWidget(&mPlaceholder);
        mContentLayout.addWidget(&mLicensesLabel);
        mContentLayout.addWidget(&mTextedit);
    }
}
