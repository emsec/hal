#include "gui/grouping/grouping_manager_widget.h"

#include "gui/gui_globals.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/grouping/grouping_color_delegate.h"
#include "gui/input_dialog/input_dialog.h"

//#include "gui/graph_widget/graph_widget.h"


#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"
#include <QAction>
#include <QMenu>
#include <QResizeEvent>
#include <QSize>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QColorDialog>
#include <QStringList>

namespace hal
{
    GroupingManagerWidget::GroupingManagerWidget(GraphTabWidget* tab_view, QWidget* parent)
        : ContentWidget("Groupings", parent),
          m_new_grouping_action(new QAction(this)),
          m_rename_action(new QAction(this)),
          m_color_select_action(new QAction(this)),
          m_delete_action(new QAction(this)),
          m_to_selection_action(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();
        m_tab_view = tab_view;

        m_new_grouping_action->setIcon(gui_utility::get_styled_svg_icon(m_new_grouping_icon_style, m_new_grouping_icon_path));
        m_rename_action->setIcon(gui_utility::get_styled_svg_icon(m_rename_grouping_icon_style, m_rename_grouping_icon_path));
        m_delete_action->setIcon(gui_utility::get_styled_svg_icon(m_delete_icon_style, m_delete_icon_path));
        m_color_select_action->setIcon(gui_utility::get_styled_svg_icon(m_color_select_icon_style, m_color_select_icon_path));
        m_to_selection_action->setIcon(gui_utility::get_styled_svg_icon(m_to_selection_icon_style, m_to_selection_icon_path));

        m_new_grouping_action->setToolTip("New");
        m_rename_action->setToolTip("Rename");
        m_color_select_action->setToolTip("Color");
        m_delete_action->setToolTip("Delete");
        m_to_selection_action->setToolTip("To selection");

        m_new_grouping_action->setText("Create New Grouping");
        m_rename_action->setText("Rename Grouping");
        m_color_select_action->setText("Select Color for Grouping");
        m_delete_action->setText("Delete View");
        m_to_selection_action->setText("Add grouping to selection");

        //m_open_action->setEnabled(false);
        //m_rename_action->setEnabled(false);
        //m_delete_action->setEnabled(false);

        mGroupingTableModel = new GroupingTableModel;

        mGroupingTableView = new QTableView(this);
        mGroupingTableView->setModel(mGroupingTableModel);
        mGroupingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGroupingTableView->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        mGroupingTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        mGroupingTableView->verticalHeader()->hide();
        mGroupingTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mGroupingTableView->setItemDelegateForColumn(2,new GroupingColorDelegate(mGroupingTableView));

        mGroupingTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);
        QFont font = mGroupingTableView->horizontalHeader()->font();
        font.setBold(true);
        mGroupingTableView->horizontalHeader()->setFont(font);
        m_content_layout->addWidget(mGroupingTableView);

        connect(m_new_grouping_action, &QAction::triggered, this, &GroupingManagerWidget::handleCreateGroupingClicked);
        connect(m_rename_action, &QAction::triggered, this, &GroupingManagerWidget::handleRenameGroupingClicked);
        connect(m_color_select_action, &QAction::triggered, this, &GroupingManagerWidget::handleColorSelectClicked);
        connect(m_to_selection_action, &QAction::triggered, this, &GroupingManagerWidget::handleToSelectionClicked);
        connect(m_delete_action, &QAction::triggered, this, &GroupingManagerWidget::handleDeleteGroupingClicked);

        connect(mGroupingTableView, &QTableView::customContextMenuRequested, this, &GroupingManagerWidget::handle_context_menu_request);
        connect(mGroupingTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GroupingManagerWidget::handleCurrentChanged);
        connect(mGroupingTableModel, &GroupingTableModel::lastEntryDeleted, this, &GroupingManagerWidget::handleLastEntryDeleted);
        handleCurrentChanged();
    }

    void GroupingManagerWidget::handleCreateGroupingClicked()
    {
        mGroupingTableModel->addDefaultEntry();
    }

    void GroupingManagerWidget::handleColorSelectClicked()
    {
        QModelIndex currentIndex = mGroupingTableView->currentIndex();
        if (!currentIndex.isValid()) return;
        QModelIndex nameIndex = mGroupingTableModel->index(currentIndex.row(),0);
        QString name = mGroupingTableModel->data(nameIndex,Qt::DisplayRole).toString();
        QModelIndex modelIndex = mGroupingTableModel->index(currentIndex.row(),2);
        QColor color = mGroupingTableModel->data(modelIndex,Qt::BackgroundRole).value<QColor>();
        color = QColorDialog::getColor(color,this,"Select color for grouping " + name);
        if (color.isValid())
            mGroupingTableModel->setData(modelIndex,color,Qt::EditRole);
    }

    void GroupingManagerWidget::handleToSelectionClicked()
    {
        QModelIndex currentIndex = mGroupingTableView->currentIndex();
        if (!currentIndex.isValid()) return;
        Grouping* grp = getCurrentGrouping().grouping();
        if (!grp) return;
        for (Module* m : grp->get_modules())
            g_selection_relay->m_selected_modules.insert(m->get_id());
        for (Gate* g : grp->get_gates())
            g_selection_relay->m_selected_gates.insert(g->get_id());
        for (Net* n : grp->get_nets())
            g_selection_relay->m_selected_nets.insert(n->get_id());
        g_selection_relay->relay_selection_changed(this);
    }

    void GroupingManagerWidget::handleRenameGroupingClicked()
    {
        QModelIndex currentIndex = mGroupingTableView->currentIndex();
        if (!currentIndex.isValid()) return;
        QModelIndex modelIndex = mGroupingTableModel->index(currentIndex.row(),0);

        InputDialog ipd;
        ipd.set_window_title("Rename Grouping");
        ipd.set_info_text("Please select a new unique name for the grouping.");
        QString oldName = mGroupingTableModel->data(modelIndex,Qt::DisplayRole).toString();
        mGroupingTableModel->setAboutToRename(oldName);
        ipd.set_input_text(oldName);
        ipd.add_validator(mGroupingTableModel);

        if (ipd.exec() == QDialog::Accepted)
            mGroupingTableModel->renameGrouping(modelIndex.row(),ipd.text_value());
        mGroupingTableModel->setAboutToRename(QString());
    }

    void GroupingManagerWidget::handleDeleteGroupingClicked()
    {
        mGroupingTableModel->removeRows(mGroupingTableView->currentIndex().row());
    }

    void GroupingManagerWidget::handle_selection_changed(const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(deselected);

        if(selected.indexes().isEmpty())
            set_toolbar_buttons_enabled(false);
        else
            set_toolbar_buttons_enabled(true);
    }

    void GroupingManagerWidget::handle_context_menu_request(const QPoint& point)
    {
        const QModelIndex clicked_index = mGroupingTableView->indexAt(point);

        QMenu context_menu;

        context_menu.addAction(m_new_grouping_action);

        if (clicked_index.isValid())
        {
            context_menu.addAction(m_rename_action);
            context_menu.addAction(m_color_select_action);
            context_menu.addAction(m_to_selection_action);
            context_menu.addAction(m_delete_action);
        }

        context_menu.exec(mGroupingTableView->viewport()->mapToGlobal(point));
    }

    GroupingTableEntry GroupingManagerWidget::getCurrentGrouping()
    {
        QModelIndex modelIndex = mGroupingTableView->currentIndex();

        return mGroupingTableModel->groupingAt(modelIndex.row());
    }

    void GroupingManagerWidget::setup_toolbar(Toolbar* toolbar)
    {
        toolbar->addAction(m_new_grouping_action);
        toolbar->addAction(m_rename_action);
        toolbar->addAction(m_color_select_action);
        toolbar->addAction(m_to_selection_action);
        toolbar->addAction(m_delete_action);
    }

    void GroupingManagerWidget::set_toolbar_buttons_enabled(bool enabled)
    {
        m_rename_action->setEnabled(enabled);
        m_color_select_action->setEnabled(enabled);
        m_to_selection_action->setEnabled(enabled);
        m_delete_action->setEnabled(enabled);
    }

    QList<QShortcut*> GroupingManagerWidget::create_shortcuts()
    {
        QList<QShortcut*> list;
        return list;
    }

    void GroupingManagerWidget::handleLastEntryDeleted()
    {
        if (mGroupingTableModel->rowCount())
            mGroupingTableView->setCurrentIndex(mGroupingTableModel->index(0,0));
        else
            handleCurrentChanged();
    }

    void GroupingManagerWidget::handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        bool enable = mGroupingTableModel->rowCount() > 0 && current.isValid();
        QAction* entryBasedAction[] = { m_rename_action, m_color_select_action,
                                        m_delete_action, m_to_selection_action, nullptr};

        QStringList iconPath, iconStyle;
        iconPath << m_rename_grouping_icon_path << m_color_select_icon_path
                             << m_delete_icon_path << m_to_selection_icon_path;
        iconStyle << m_rename_grouping_icon_style << m_color_select_icon_style
                              << m_delete_icon_style << m_to_selection_icon_style;

        for (int iacc = 0; entryBasedAction[iacc]; iacc++)
        {
            entryBasedAction[iacc]->setEnabled(enable);
            entryBasedAction[iacc]->setIcon(
                        gui_utility::get_styled_svg_icon(enable
                                                         ? iconStyle.at(iacc)
                                                         : disabled_icon_style(),
                                                         iconPath.at(iacc)));
        }
    }

    QString GroupingManagerWidget::disabled_icon_style() const
    {
        return m_disabled_icon_style;
    }

    QString GroupingManagerWidget::new_grouping_icon_path() const
    {
        return m_new_grouping_icon_path;
    }

    QString GroupingManagerWidget::new_grouping_icon_style() const
    {
        return m_new_grouping_icon_style;
    }

    QString GroupingManagerWidget::rename_grouping_icon_path() const
    {
        return m_rename_grouping_icon_path;
    }

    QString GroupingManagerWidget::rename_grouping_icon_style() const
    {
        return m_rename_grouping_icon_style;
    }

    QString GroupingManagerWidget::delete_icon_path() const
    {
        return m_delete_icon_path;
    }

    QString GroupingManagerWidget::delete_icon_style() const
    {
        return m_delete_icon_style;
    }

    QString GroupingManagerWidget::color_select_icon_path() const
    {
        return m_color_select_icon_path;
    }

    QString GroupingManagerWidget::color_select_icon_style() const
    {
        return m_color_select_icon_style;
    }

    QString GroupingManagerWidget::to_selection_icon_path() const
    {
        return m_to_selection_icon_path;
    }

    QString GroupingManagerWidget::to_selection_icon_style() const
    {
        return m_to_selection_icon_style;
    }

    void GroupingManagerWidget::set_disabled_icon_style(const QString& style)
    {
        m_disabled_icon_style = style;
    }

    void GroupingManagerWidget::set_new_grouping_icon_path(const QString& path)
    {
        m_new_grouping_icon_path = path;
    }

    void GroupingManagerWidget::set_new_grouping_icon_style(const QString& style)
    {
        m_new_grouping_icon_style = style;
    }

    void GroupingManagerWidget::set_rename_grouping_icon_path(const QString& path)
    {
        m_rename_grouping_icon_path = path;
    }

    void GroupingManagerWidget::set_rename_grouping_icon_style(const QString& style)
    {
        m_rename_grouping_icon_style = style;
    }

    void GroupingManagerWidget::set_delete_icon_path(const QString& path)
    {
        m_delete_icon_path = path;
    }

    void GroupingManagerWidget::set_delete_icon_style(const QString& style)
    {
        m_delete_icon_style = style;
    }

    void GroupingManagerWidget::set_color_select_icon_path(const QString& path)
    {
        m_color_select_icon_path = path;
    }

    void GroupingManagerWidget::set_color_select_icon_style(const QString& style)
    {
        m_color_select_icon_style = style;
    }

    void GroupingManagerWidget::set_to_selection_icon_path(const QString& path)
    {
        m_to_selection_icon_path = path;
    }

    void GroupingManagerWidget::set_to_selection_icon_style(const QString& style)
    {
        m_to_selection_icon_style = style;
    }
}
