#include "gui/context_manager_widget/context_manager_widget.h"

#include "gui/gui_globals.h"

#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
//#include "gui/graph_widget/graph_widget.h"

#include "gui/input_dialog/input_dialog.h"
#include "gui/validator/empty_string_validator.h"
#include "gui/validator/unique_string_validator.h"

//#include "hal_core/utilities/log.h"
#include "gui/gui_utils/graphics.h"
//#include "hal_core/netlist/gate.h"
//#include "hal_core/netlist/module.h"
//#include "hal_core/netlist/netlist.h"
#include "gui/toolbar/toolbar.h"
#include <QAction>
#include <QDebug>
//#include <QListWidgetItem>
#include <QMenu>
#include <QResizeEvent>
#include <QSize>
//#include <QStringList>
//#include <QStringListModel>
#include <QVBoxLayout>
#include <QHeaderView>
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/user_action_compound.h"
#include <QShortcut>

namespace hal
{
    ContextManagerWidget::ContextManagerWidget(GraphTabWidget* tab_view, QWidget* parent)
        : ContentWidget("Views", parent), mSearchbar(new Searchbar(this)), mNewViewAction(new QAction(this)), mRenameAction(new QAction(this)), mDuplicateAction(new QAction(this)),
          mDeleteAction(new QAction(this)), mOpenAction(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();
        mTabView = tab_view;

        mOpenAction->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mNewViewAction->setIcon(gui_utility::getStyledSvgIcon(mNewViewIconStyle, mNewViewIconPath));
        mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mRenameIconStyle, mRenameIconPath));
        mDuplicateAction->setIcon(gui_utility::getStyledSvgIcon(mDuplicateIconStyle, mDuplicateIconPath));
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(mDeleteIconStyle, mDeleteIconPath));
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));

        mOpenAction->setToolTip("Open");
        mNewViewAction->setToolTip("New");
        mRenameAction->setToolTip("Rename");
        mDuplicateAction->setToolTip("Duplicate");
        mDeleteAction->setToolTip("Delete");
        mSearchAction->setToolTip("Search");

        mOpenAction->setText("Open view");
        mNewViewAction->setText("Create new view");
        mRenameAction->setText("Rename view");
        mDuplicateAction->setText("Duplicate view");
        mDeleteAction->setText("Delete view");
        mSearchAction->setText("Search");

        //mOpenAction->setEnabled(false);
        //mRenameAction->setEnabled(false);
        //mDuplicateAction->setEnabled(false);
        //mDeleteAction->setEnabled(false);

        mContextTableModel = gGraphContextManager->getContextTableModel();

        mContextTableProxyModel = new ContextTableProxyModel();
        mContextTableProxyModel->setSourceModel(mContextTableModel);
        mContextTableProxyModel->setSortRole(Qt::UserRole);

        mContextTableView = new QTableView(this);
        mContextTableView->setModel(mContextTableProxyModel);
        mContextTableView->setSortingEnabled(true);
        mContextTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mContextTableView->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        mContextTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        mContextTableView->sortByColumn(1, Qt::SortOrder::DescendingOrder);
        mContextTableView->verticalHeader()->hide();
        mContextTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        mContextTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        mContextTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);


        mContentLayout->addWidget(mContextTableView);
        mContentLayout->addWidget(mSearchbar);

        mSearchbar->hide();
        enableSearchbar(mContextTableProxyModel->rowCount() > 0);

        connect(mOpenAction, &QAction::triggered, this, &ContextManagerWidget::handleOpenContextClicked);
        connect(mNewViewAction, &QAction::triggered, this, &ContextManagerWidget::handleCreateContextClicked);
        connect(mRenameAction, &QAction::triggered, this, &ContextManagerWidget::handleRenameContextClicked);
        connect(mDuplicateAction, &QAction::triggered, this, &ContextManagerWidget::handleDuplicateContextClicked);
        connect(mDeleteAction, &QAction::triggered, this, &ContextManagerWidget::handleDeleteContextClicked);
        connect(mSearchAction, &QAction::triggered, this, &ContextManagerWidget::toggleSearchbar);

        connect(mContextTableView, &QTableView::customContextMenuRequested, this, &ContextManagerWidget::handleContextMenuRequest);
        connect(mContextTableView, &QTableView::doubleClicked, this, &ContextManagerWidget::handleOpenContextClicked);
        connect(mContextTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContextManagerWidget::handleSelectionChanged);
        connect(mContextTableModel, &ContextTableModel::rowsRemoved, this, &ContextManagerWidget::handleDataChanged);
        connect(mContextTableModel, &ContextTableModel::rowsInserted, this, &ContextManagerWidget::handleDataChanged);

        connect(mSearchbar, &Searchbar::textEdited, mContextTableProxyModel, &ContextTableProxyModel::handleFilterTextChanged);
        connect(mSearchbar, &Searchbar::textEdited, this, &ContextManagerWidget::updateSearchIcon);
    }

    void ContextManagerWidget::handleCreateContextClicked()
    {
        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::Context,
                  QString::fromStdString(gNetlist->get_top_module()->get_name())));
        act->addAction(new ActionAddItemsToObject({gNetlist->get_top_module()->get_id()}, {}));
        act->exec();
    }

    void ContextManagerWidget::handleOpenContextClicked()
    {
        GraphContext* clicked_context = getCurrentContext();
        mTabView->showContext(clicked_context);
    }

    void ContextManagerWidget::handleRenameContextClicked()
    {
        GraphContext* clicked_context = getCurrentContext();

        QStringList used_context_names;
        for (const auto& context : gGraphContextManager->getContexts())
            used_context_names.append(context->name());

        UniqueStringValidator unique_validator(used_context_names);
        EmptyStringValidator empty_validator;

        InputDialog ipd;
        ipd.setWindowTitle("Rename View");
        ipd.setInfoText("Please select a new unique name for the view.");
        ipd.setInputText(clicked_context->name());
        ipd.addValidator(&unique_validator);
        ipd.addValidator(&empty_validator);

        if (ipd.exec() == QDialog::Accepted)
        {
            ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
            act->setObject(UserActionObject(clicked_context->id(),UserActionObjectType::Context));
            act->exec();
        }
    }

    void ContextManagerWidget::handleDuplicateContextClicked()
    {
        GraphContext* clicked_context = getCurrentContext();
        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::Context,clicked_context->name() + " (Copy)"));
        act->addAction(new ActionAddItemsToObject(clicked_context->modules(),clicked_context->gates()));
        act->exec();
    }

    void ContextManagerWidget::handleDeleteContextClicked()
    {
        GraphContext* clicked_context = getCurrentContext();
        ActionDeleteObject* act = new ActionDeleteObject;
        act->setObject(UserActionObject(clicked_context->id(),UserActionObjectType::Context));
        act->exec();
    }

    void ContextManagerWidget::handleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(deselected);

        if(selected.indexes().isEmpty())
            setToolbarButtonsEnabled(false);
        else
            setToolbarButtonsEnabled(true);
    }

    void ContextManagerWidget::handleContextMenuRequest(const QPoint& point)
    {
        const QModelIndex clicked_index = mContextTableView->indexAt(point);

        QMenu context_menu;

        context_menu.addAction(mNewViewAction);

        if (clicked_index.isValid())
        {
            context_menu.addAction(mOpenAction);
            context_menu.addAction(mDuplicateAction);
            context_menu.addAction(mRenameAction);
            context_menu.addAction(mDeleteAction);
        }

        context_menu.exec(mContextTableView->viewport()->mapToGlobal(point));
    }

    void ContextManagerWidget::handleDataChanged()
    {
        enableSearchbar(mContextTableProxyModel->rowCount() > 0);
    }

    void ContextManagerWidget::updateSearchIcon()
    {
        if (mSearchbar->filterApplied() && mSearchbar->isVisible())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    void ContextManagerWidget::selectViewContext(GraphContext* context)
    {
        const QModelIndex source_model_index = mContextTableModel->getIndex(context);
        const QModelIndex proxy_model_index = mContextTableProxyModel->mapFromSource(source_model_index);

        if(proxy_model_index.isValid())
            mContextTableView->setCurrentIndex(proxy_model_index);
        else
            mContextTableView->clearSelection();
    }

    GraphContext* ContextManagerWidget::getCurrentContext()
    {
        QModelIndex proxy_model_index = mContextTableView->currentIndex();
        QModelIndex source_model_index = mContextTableProxyModel->mapToSource(proxy_model_index);

        return mContextTableModel->getContext(source_model_index);
    }

    void ContextManagerWidget::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mNewViewAction);
        toolbar->addAction(mOpenAction);
        toolbar->addAction(mDuplicateAction);
        toolbar->addAction(mRenameAction);
        toolbar->addAction(mDeleteAction);
        toolbar->addAction(mSearchAction);
    }

    void ContextManagerWidget::enableSearchbar(bool enable)
    {
        QString iconStyle = enable ? mSearchIconStyle : mDisabledIconStyle;
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mSearchIconPath));
        if (!enable && mSearchbar->isVisible())
        {
            mSearchbar->hide();
            setFocus();
        }
        mSearchAction->setEnabled(enable);
    }

    void ContextManagerWidget::setToolbarButtonsEnabled(bool enabled)
    {
        mOpenAction->setEnabled(enabled);
        mRenameAction->setEnabled(enabled);
        mDuplicateAction->setEnabled(enabled);
        mDeleteAction->setEnabled(enabled);

        if(enabled)
        {
            mOpenAction->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
            mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mRenameIconStyle, mRenameIconPath));
            mDuplicateAction->setIcon(gui_utility::getStyledSvgIcon(mDuplicateIconStyle, mDuplicateIconPath));
            mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(mDeleteIconStyle, mDeleteIconPath));
        }
        else
        {
            mOpenAction->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mOpenIconPath));
            mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mRenameIconPath));
            mDuplicateAction->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mDuplicateIconPath));
            mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mDeleteIconPath));
        }
    }

    QList<QShortcut*> ContextManagerWidget::createShortcuts()
    {
        mSearchShortcut = new QShortcut(mSearchKeysequence, this);
        connect(mSearchShortcut, &QShortcut::activated, mSearchAction, &QAction::trigger);

        QList<QShortcut*> list;
        list.append(mSearchShortcut);

        return list;
    }

    void ContextManagerWidget::toggleSearchbar()
    {
        if (!mSearchAction->isEnabled())
            return;

        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            setFocus();
        }
    }

    QString ContextManagerWidget::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    QString ContextManagerWidget::newViewIconPath() const
    {
        return mNewViewIconPath;
    }

    QString ContextManagerWidget::newViewIconStyle() const
    {
        return mNewViewIconStyle;
    }

    QString ContextManagerWidget::renameIconPath() const
    {
        return mRenameIconPath;
    }

    QString ContextManagerWidget::renameIconStyle() const
    {
        return mRenameIconStyle;
    }

    QString ContextManagerWidget::duplicateIconPath() const
    {
        return mDuplicateIconPath;
    }

    QString ContextManagerWidget::duplicateIconStyle() const
    {
        return mDuplicateIconStyle;
    }

    QString ContextManagerWidget::deleteIconPath() const
    {
        return mDeleteIconPath;
    }

    QString ContextManagerWidget::deleteIconStyle() const
    {
        return mDeleteIconStyle;
    }

    QString ContextManagerWidget::openIconPath() const
    {
        return mOpenIconPath;
    }

    QString ContextManagerWidget::openIconStyle() const
    {
        return mOpenIconStyle;
    }

    QString ContextManagerWidget::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString ContextManagerWidget::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString ContextManagerWidget::searchActiveIconStyle() const
    {
        return mSearchActiveIconStyle;
    }

    void ContextManagerWidget::setDisabledIconStyle(const QString& style)
    {
        mDisabledIconStyle = style;
    }

    void ContextManagerWidget::setNewViewIconPath(const QString& path)
    {
        mNewViewIconPath = path;
    }

    void ContextManagerWidget::setNewViewIconStyle(const QString& style)
    {
        mNewViewIconStyle = style;
    }

    void ContextManagerWidget::setRenameIconPath(const QString& path)
    {
        mRenameIconPath = path;
    }

    void ContextManagerWidget::setRenameIconStyle(const QString& style)
    {
        mRenameIconStyle = style;
    }

    void ContextManagerWidget::setDuplicateIconPath(const QString& path)
    {
        mDuplicateIconPath = path;
    }

    void ContextManagerWidget::setDuplicateIconStyle(const QString& style)
    {
        mDuplicateIconStyle = style;
    }

    void ContextManagerWidget::setDeleteIconPath(const QString& path)
    {
        mDeleteIconPath = path;
    }

    void ContextManagerWidget::setDeleteIconStyle(const QString& style)
    {
        mDeleteIconStyle = style;
    }

    void ContextManagerWidget::setOpenIconPath(const QString& path)
    {
        mOpenIconPath = path;
    }

    void ContextManagerWidget::setOpenIconStyle(const QString& style)
    {
        mOpenIconStyle = style;
    }

    void ContextManagerWidget::setSearchIconPath(const QString& path)
    {
        mSearchIconPath = path;
    }

    void ContextManagerWidget::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }

    void ContextManagerWidget::setSearchActiveIconStyle(const QString& style)
    {
        mSearchActiveIconStyle = style;
    }
}
