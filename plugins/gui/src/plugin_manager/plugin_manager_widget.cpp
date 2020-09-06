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
        m_content_layout->addWidget(&m_table_view);
        m_table_view.setSelectionBehavior(QAbstractItemView::SelectRows);
        m_add_action    = new QAction("Add", nullptr);
        m_remove_action = new QAction("Remove", nullptr);
        m_table_view.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_table_view.setAlternatingRowColors(true);
        m_table_view.horizontalHeader()->setStretchLastSection(true);
        m_table_view.horizontalHeader()->setHighlightSections(false);
        m_table_view.verticalHeader()->hide();
        m_table_view.setFocusPolicy(Qt::NoFocus);
        m_table_view.setAcceptDrops(true);
        m_table_view.setDragEnabled(true);
        m_table_view.setDragDropMode(QAbstractItemView::InternalMove);
        m_table_view.setShowGrid(false);
        connect(m_add_action, SIGNAL(triggered()), this, SLOT(on_action_add_triggered()));
        connect(m_remove_action, SIGNAL(triggered()), this, SLOT(on_action_remove_triggered()));
    }

    void PluginManagerWidget::set_plugin_model(PluginModel* m_plugin_model)
    {
        m_table_view.setModel(m_plugin_model);
        m_model = m_plugin_model;
        connect(&m_table_view, &QTableView::doubleClicked, m_model, &PluginModel::handle_run_plugin_triggered);
    }

    void PluginManagerWidget::on_action_add_triggered()
    {
        QString file_name = QFileDialog::getOpenFileName(this, tr("Open Plugin"), QDir::currentPath());
        QFileInfo fi(file_name);
        QString name = fi.baseName();
        if (name.isEmpty())
            return;
        m_model->request_load_plugin(name, file_name);
        m_table_view.resizeColumnsToContents();
    }

    void PluginManagerWidget::on_action_remove_triggered()
    {
        QModelIndexList index = m_table_view.selectionModel()->selectedIndexes();
        if (index.isEmpty())
            return;
        m_model->request_unload_plugin(index);
        m_table_view.resizeColumnsToContents();
    }

    void PluginManagerWidget::setup_toolbar(Toolbar* Toolbar)
    {
        Toolbar->addAction(m_add_action);
        Toolbar->addAction(m_remove_action);
    }
}
