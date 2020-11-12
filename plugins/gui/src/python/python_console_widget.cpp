#include "gui/python/python_console_widget.h"

#include "gui/python/python_console.h"

#include <QVBoxLayout>

namespace hal
{
    PythonConsoleWidget::PythonConsoleWidget(QWidget* parent) : ContentWidget("Python Console", parent), mConsole(new PythonConsole())
    {
        mContentLayout->addWidget(mConsole);
    }

    void PythonConsoleWidget::setupToolbar(Toolbar* Toolbar)
    {
        Q_UNUSED(Toolbar);
    }

    QList<QShortcut*> PythonConsoleWidget::createShortcuts()
    {
        QList<QShortcut*> list;
        return list;
    }
}
