#include "python/python_console_widget.h"

#include "python/python_console.h"

#include <QVBoxLayout>
namespace hal{
python_console_widget::python_console_widget(QWidget* parent) : content_widget("Python Console", parent), m_console(new python_console())
{
    m_content_layout->addWidget(m_console);
}

void python_console_widget::setup_toolbar(toolbar* toolbar)
{
    Q_UNUSED(toolbar);
}

QList<QShortcut*> python_console_widget::create_shortcuts()
{
    QList<QShortcut*> list;
    return list;
}
}
