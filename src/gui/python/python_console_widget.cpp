#include "python/python_console_widget.h"

#include "python/python_console.h"

#include <QVBoxLayout>

namespace hal
{
    PythonConsoleWidget::PythonConsoleWidget(QWidget* parent) : ContentWidget("Python Console", parent), m_console(new PythonConsole())
    {
        m_content_layout->addWidget(m_console);
    }

    void PythonConsoleWidget::setup_toolbar(toolbar* toolbar)
    {
        Q_UNUSED(toolbar);
    }

    QList<QShortcut*> PythonConsoleWidget::create_shortcuts()
    {
        QList<QShortcut*> list;
        return list;
    }
}
