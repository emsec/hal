#include "main_window/about_dialog.h"
#include <core/utils.h>

namespace hal
{
    about_dialog::about_dialog(QWidget* parent) : QDialog(parent), m_placeholder("about gui"), m_licenses_label("Open Source licenses:"), m_textedit()
    {
        setLayout(&m_content_layout);
        m_placeholder.setText(QString("Compiled with Qt Version ") + QString(QT_VERSION_STR));
        m_textedit.setPlainText(QString::fromStdString(core_utils::get_open_source_licenses()));
        m_textedit.setReadOnly(true);
        m_content_layout.addWidget(&m_placeholder);
        m_content_layout.addWidget(&m_licenses_label);
        m_content_layout.addWidget(&m_textedit);
    }
}
