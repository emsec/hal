#include "gui/gui_utils/special_log_content_manager.h"
#include "gui/main_window/main_window.h"
#include "gui/python/python_editor.h"

#include <QTimer>
#include <QDateTime>

namespace hal
{
    SpecialLogContentManager::SpecialLogContentManager() : m_timer(new QTimer(this))
    {

    }

    SpecialLogContentManager::SpecialLogContentManager(MainWindow *main_window, PythonEditor *python_editor) : m_main_window(main_window), m_python_editor(python_editor)
    {

    }

    SpecialLogContentManager::~SpecialLogContentManager()
    {

    }

}
