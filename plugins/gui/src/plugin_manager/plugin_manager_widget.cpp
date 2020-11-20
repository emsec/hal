#include "gui/plugin_manager/plugin_manager_widget.h"

#include "hal_core/utilities/log.h"
#include "gui/plugin_manager/plugin_model.h"
#include "gui/toolbar/toolbar.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QVBoxLayout>

namespace hal
{
    PluginManagerWidget::PluginManagerWidget(QWidget* parent) : ContentWidget("plugins", parent)
    {
        mContentLayout->addWidget(&mTableView);
        mTableView.setSelectionBehavior(QAbstractItemView::SelectRows);
        mAddAction    = new QAction("Add", nullptr);
        mRemoveAction = new QAction("Remove", nullptr);
        mTableView.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mTableView.setAlternatingRowColors(true);
        mTableView.horizontalHeader()->setStretchLastSection(true);
        mTableView.horizontalHeader()->setHighlightSections(false);
        mTableView.verticalHeader()->hide();
        mTableView.setFocusPolicy(Qt::NoFocus);
        mTableView.setAcceptDrops(true);
        mTableView.setDragEnabled(true);
        mTableView.setDragDropMode(QAbstractItemView::InternalMove);
        mTableView.setShowGrid(false);
        connect(mAddAction, SIGNAL(triggered()), this, SLOT(onActionAddTriggered()));
        connect(mRemoveAction, SIGNAL(triggered()), this, SLOT(onActionRemoveTriggered()));
    }

    void PluginManagerWidget::setPluginModel(PluginModel* mPluginModel)
    {
        mTableView.setModel(mPluginModel);
        mModel = mPluginModel;
        connect(&mTableView, &QTableView::doubleClicked, mModel, &PluginModel::handleRunPluginTriggered);
    }

    void PluginManagerWidget::onActionAddTriggered()
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Plugin"), QDir::currentPath());
        QFileInfo fi(fileName);
        QString name = fi.baseName();
        if (name.isEmpty())
            return;
        mModel->requestLoadPlugin(name, fileName);
        mTableView.resizeColumnsToContents();
    }

    void PluginManagerWidget::onActionRemoveTriggered()
    {
        QModelIndexList index = mTableView.selectionModel()->selectedIndexes();
        if (index.isEmpty())
            return;
        mModel->requestUnloadPlugin(index);
        mTableView.resizeColumnsToContents();
    }

    void PluginManagerWidget::setupToolbar(Toolbar* Toolbar)
    {
        Toolbar->addAction(mAddAction);
        Toolbar->addAction(mRemoveAction);
    }
}
