#include "gui/main_window/about_dialog.h"

#include "hal_core/utilities/utils.h"
#include "hal_version.h"

namespace hal
{
    AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent)
    {
        setWindowTitle("About:");

        // remove questionmark from title bar, it is not implemented and only confusing
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        mLayout = new QVBoxLayout(this);
        mLayout->addWidget(new QLabel(QString("hal %1.%2.%3 (%4)")
                                      .arg(hal_version::major)
                                      .arg(hal_version::minor)
                                      .arg(hal_version::patch)
                                      .arg(hal_version::version.c_str())
                                      ,this));
        mLayout->addWidget(new QLabel(QString("Compiled with Qt Version %1")
                                      .arg(QT_VERSION_STR)
                                      ,this));
        mLayout->addWidget(new QLabel(QString("Open Source licenses:"),this));
        mTextedit = new QPlainTextEdit(QString::fromStdString(utils::get_open_source_licenses()),this);
        mTextedit->setReadOnly(true);
        mLayout->addWidget(mTextedit);
    }
}
