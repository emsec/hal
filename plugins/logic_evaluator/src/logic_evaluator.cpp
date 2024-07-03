#include "logic_evaluator/logic_evaluator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"

#include <QFile>
#include <QDate>
#include <QColor>
#include <QFileDialog>
#include <QStatusBar>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QInputDialog>
#include <QTabBar>
#include "hal_core/plugin_system/plugin_manager.h"

namespace hal
{


    LogicEvaluator::LogicEvaluator(QWidget *parent)
        : QObject(parent)
    {
    }

    LogicEvaluator::~LogicEvaluator()
    {
    }

    void LogicEvaluator::setGates(std::vector<Gate*> gates)
    {
    }

}    // namespace hal
