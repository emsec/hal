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
#include <QApplication>
#include <QInputDialog>

namespace hal
{
    ContextManagerWidget::ContextManagerWidget(GraphTabWidget* tab_view, QWidget* parent)
        : ContentWidget("Views", parent), mSearchbar(new Searchbar(this)), mNewDirectoryAction(new QAction(this)), mNewViewAction(new QAction(this)), mRenameAction(new QAction(this)), mDuplicateAction(new QAction(this)),
          mDeleteAction(new QAction(this)), mOpenAction(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();
        mTabView = tab_view;

        mOpenAction->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mNewViewAction->setIcon(gui_utility::getStyledSvgIcon(mNewViewIconStyle, mNewViewIconPath));
        mNewDirectoryAction->setIcon(gui_utility::getStyledSvgIcon(mNewViewIconStyle, mNewDirIconPath));
        mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mRenameIconStyle, mRenameIconPath));
        mDuplicateAction->setIcon(gui_utility::getStyledSvgIcon(mDuplicateIconStyle, mDuplicateIconPath));
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(mDeleteIconStyle, mDeleteIconPath));
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));

        mOpenAction->setToolTip("Open");
        mNewViewAction->setToolTip("New View");
        mNewDirectoryAction->setToolTip("New Directory");
        mRenameAction->setToolTip("Rename view");
        mDuplicateAction->setToolTip("Duplicate");
        mDeleteAction->setToolTip("Delete");
        mSearchAction->setToolTip("Search");

        mOpenAction->setText("Open view");
        mNewViewAction->setText("Create new view");
        mNewDirectoryAction->setText("Create new Directory");
        mRenameAction->setText("Rename view");
        mDuplicateAction->setText("Duplicate view");
        mDeleteAction->setText("Delete view");
        mSearchAction->setText("Search");

        //mOpenAction->setEnabled(false);
        //mRenameAction->setEnabled(false);
        //mDuplicateAction->setEnabled(false);
        //mDeleteAction->setEnabled(false);

        mContextTreeModel = gGraphContextManager->getContextTreeModel();

        mContextTreeProxyModel = new ContextProxyModel(this);
        mContextTreeProxyModel->setSourceModel(mContextTreeModel);
        mContextTreeProxyModel->setSortRole(Qt::UserRole);

        mContextTreeView = new QTreeView(this);
        mContextTreeView->setModel(mContextTreeProxyModel);
        mContextTreeView->setSortingEnabled(true);
        mContextTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mContextTreeView->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        mContextTreeView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        mContextTreeView->sortByColumn(1, Qt::SortOrder::DescendingOrder);
        mContextTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);



        mContentLayout->addWidget(mContextTreeView);
        mContentLayout->addWidget(mSearchbar);

        mSearchbar->hide();
        mSearchbar->setColumnNames(mContextTreeProxyModel->getColumnNames());
        enableSearchbar(mContextTreeProxyModel->rowCount() > 0);

        connect(mOpenAction, &QAction::triggered, this, &ContextManagerWidget::handleOpenContextClicked);
        connect(mNewViewAction, &QAction::triggered, this, &ContextManagerWidget::handleCreateContextClicked);
        connect(mNewDirectoryAction, &QAction::triggered, this, &ContextManagerWidget::handleCreateDirectoryClicked);
        connect(mRenameAction, &QAction::triggered, this, &ContextManagerWidget::handleRenameClicked);
        connect(mDuplicateAction, &QAction::triggered, this, &ContextManagerWidget::handleDuplicateContextClicked);
        connect(mDeleteAction, &QAction::triggered, this, &ContextManagerWidget::handleDeleteClicked);
        connect(mSearchAction, &QAction::triggered, this, &ContextManagerWidget::toggleSearchbar);

        connect(mContextTreeView, &QTreeView::customContextMenuRequested, this, &ContextManagerWidget::handleContextMenuRequest);
        connect(mContextTreeView, &QTreeView::doubleClicked, this, &ContextManagerWidget::handleItemDoubleClicked);
        connect(mContextTreeView, &QTreeView::clicked, this, &ContextManagerWidget::handleItemClicked);
        connect(mContextTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContextManagerWidget::handleSelectionChanged);
        connect(mContextTreeModel, &ContextTreeModel::rowsRemoved, this, &ContextManagerWidget::handleDataChanged);
        connect(mContextTreeModel, &ContextTreeModel::rowsInserted, this, &ContextManagerWidget::handleDataChanged);

        connect(mSearchbar, &Searchbar::triggerNewSearch, this, &ContextManagerWidget::updateSearchIcon);
        connect(mSearchbar, &Searchbar::triggerNewSearch, mContextTreeProxyModel, &ContextProxyModel::startSearch);

        mShortCutDeleteItem = new QShortcut(ContentManager::sSettingDeleteItem->value().toString(), this);
        mShortCutDeleteItem->setEnabled(false);

        connect(ContentManager::sSettingDeleteItem, &SettingsItemKeybind::keySequenceChanged, mShortCutDeleteItem, &QShortcut::setKey);
        connect(mShortCutDeleteItem, &QShortcut::activated, this, &ContextManagerWidget::handleDeleteClicked);

        connect(qApp, &QApplication::focusChanged, this, &ContextManagerWidget::handleFocusChanged);

        connect(mContextTreeModel, &ContextTreeModel::directoryCreatedSignal, this, &ContextManagerWidget::selectDirectory);
    }

    void ContextManagerWidget::handleCreateContextClicked()
    {
        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::ContextView,
                  QString::fromStdString(gNetlist->get_top_module()->get_name())));
        act->addAction(new ActionAddItemsToObject({gNetlist->get_top_module()->get_id()}, {}));
        act->exec();
    }

    void ContextManagerWidget::handleCreateDirectoryClicked()
    {
        bool confirm;
        QString newName = QInputDialog::getText(this, "Directory name", "name:", QLineEdit::Normal, "", &confirm);

        if (confirm && !newName.isEmpty())
        {
            UserActionCompound* act = new UserActionCompound;
            act->setUseCreatedObject();
            act->addAction(new ActionCreateObject(UserActionObjectType::ContextDir, newName));
            act->addAction(new ActionAddItemsToObject({gNetlist->get_top_module()->get_id()}, {}));
            act->exec();        }
    }

    void ContextManagerWidget::handleOpenContextClicked()
    {
        GraphContext* defaultContext = getCurrentContext();
        if (!defaultContext) return;
        mTabView->showContext(defaultContext);
    }

    void ContextManagerWidget::handleItemDoubleClicked(const QModelIndex &proxyIndex)
    {
        QModelIndex sourceIndex = mContextTreeProxyModel->mapToSource(proxyIndex);
        ContextTreeItem* item = static_cast<ContextTreeItem*>(mContextTreeModel->getItemFromIndex(sourceIndex));

        mContextTreeModel->setCurrentDirectory(item);

        if (item->isContext()) {

            GraphContext* clickedContext = item->context();

            if (!clickedContext) return;

            mTabView->showContext(clickedContext);
        }
    }

    void ContextManagerWidget::handleItemClicked(const QModelIndex &proxyIndex)
    {
        QModelIndex sourceIndex = mContextTreeProxyModel->mapToSource(proxyIndex);
        ContextTreeItem* item = static_cast<ContextTreeItem*>(mContextTreeModel->getItemFromIndex(sourceIndex));

        mContextTreeModel->setCurrentDirectory(item);

    }

    void ContextManagerWidget::handleRenameClicked()
    {
        ContextTreeItem* clicked_item = getCurrentItem();

        if (clicked_item->isContext()) {
            GraphContext* clicked_context = clicked_item->context();

            if (!clicked_context) return;

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
                act->setObject(UserActionObject(clicked_context->id(),UserActionObjectType::ContextView));
                act->exec();
                clicked_context->setExclusiveModuleId(0, false);
            }
        }
        else if (clicked_item->isDirectory()){
            ContextDirectory* clicked_directory = clicked_item->directory();

            if (!clicked_directory) return;

            InputDialog ipd;
            ipd.setWindowTitle("Rename directory");
            ipd.setInfoText("Please select a new name for the directory.");
            ipd.setInputText(clicked_directory->name());

            if (ipd.exec() == QDialog::Accepted)
            {
                ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                act->setObject(UserActionObject(clicked_directory->id(),UserActionObjectType::ContextDir));
                act->exec();
            }
        }
    }

    void ContextManagerWidget::handleDuplicateContextClicked()
    {
        GraphContext* clicked_context = getCurrentContext();

        if (!clicked_context) return;

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::ContextView,clicked_context->name() + " (Copy)"));
        act->addAction(new ActionAddItemsToObject(clicked_context->modules(),clicked_context->gates()));
        act->exec();
    }

    void ContextManagerWidget::handleDeleteClicked()
    {
        QModelIndex current = mContextTreeView->currentIndex();
        if (!current.isValid()) return;

        ContextTreeItem* currentItem = getCurrentItem();

        if (currentItem->isContext()) {
            GraphContext* clicked_context = currentItem->context();

            if (!clicked_context) return;

            ActionDeleteObject* act = new ActionDeleteObject;
            act->setObject(UserActionObject(clicked_context->id(),UserActionObjectType::ContextView));
            act->exec();
        }
        else if (currentItem->isDirectory()) {

            ContextDirectory* clicked_directory = currentItem->directory();

            if (!clicked_directory) return;

            ActionDeleteObject* act = new ActionDeleteObject;
            act->setObject(UserActionObject(clicked_directory->id(),UserActionObjectType::ContextDir));
            act->exec();
        }


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
        const QModelIndex clicked_index = mContextTreeView->indexAt(point);

        QMenu context_menu;

        context_menu.addAction(mNewViewAction);
        context_menu.addAction(mNewDirectoryAction);

        GraphContext* clicked_context = getCurrentContext();
        ContextDirectory* clicked_directory = getCurrentItem()->directory();

        if (!clicked_context && !clicked_directory) {
            context_menu.exec(mContextTreeView->viewport()->mapToGlobal(point));
            return;
        }

        if (clicked_index.isValid() && clicked_context)
        {
            context_menu.addAction(mOpenAction);
            context_menu.addAction(mDuplicateAction);
            context_menu.addAction(mRenameAction);
            context_menu.addAction(mDeleteAction);
        }

        if (clicked_index.isValid() && clicked_directory)
        {
            context_menu.addAction(mDeleteAction);
            context_menu.addAction(mRenameAction);
        }

        context_menu.exec(mContextTreeView->viewport()->mapToGlobal(point));
    }

    void ContextManagerWidget::handleDataChanged()
    {
        enableSearchbar(mContextTreeProxyModel->rowCount() > 0);
    }

    void ContextManagerWidget::updateSearchIcon()
    {
        if (mSearchbar->filterApplied() && mSearchbar->isVisible())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    void ContextManagerWidget::selectDirectory(ContextTreeItem* item)
    {
        const QModelIndex source_model_index = mContextTreeModel->getIndexFromItem(static_cast<BaseTreeItem*>(item));
        const QModelIndex proxy_model_index = mContextTreeProxyModel->mapFromSource(source_model_index);

        if(proxy_model_index.isValid())
            mContextTreeView->setCurrentIndex(proxy_model_index);
        else
            mContextTreeView->clearSelection();
    }

    void ContextManagerWidget::selectViewContext(GraphContext* context)
    {
        const QModelIndex source_model_index = mContextTreeModel->getIndexFromContext(context);
        const QModelIndex proxy_model_index = mContextTreeProxyModel->mapFromSource(source_model_index);

        if(proxy_model_index.isValid())
            mContextTreeView->setCurrentIndex(proxy_model_index);
        else
            mContextTreeView->clearSelection();
    }

    GraphContext* ContextManagerWidget::getCurrentContext()
    {
        QModelIndex proxy_model_index = mContextTreeView->currentIndex();
        QModelIndex source_model_index = mContextTreeProxyModel->mapToSource(proxy_model_index);

        return mContextTreeModel->getContext(source_model_index);
    }

    ContextTreeItem *ContextManagerWidget::getCurrentItem()
    {
        QModelIndex proxy_model_index = mContextTreeView->currentIndex();
        QModelIndex source_model_index = mContextTreeProxyModel->mapToSource(proxy_model_index);

        BaseTreeItem* currentItem = mContextTreeModel->getItemFromIndex(source_model_index);
        if (currentItem != mContextTreeModel->getRootItem())
            return static_cast<ContextTreeItem*>(currentItem);

        return nullptr;
    }

    void ContextManagerWidget::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mNewDirectoryAction);
        toolbar->addAction(mNewViewAction);
        toolbar->addAction(mOpenAction);
        toolbar->addAction(mDuplicateAction);
        toolbar->addAction(mRenameAction);
        toolbar->addAction(mRenameAction);
        toolbar->addAction(mDeleteAction);
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

    QString ContextManagerWidget::newDirIconPath() const
    {
        return mNewDirIconPath;
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

    void ContextManagerWidget::setNewDirIconPath(const QString& path)
    {
        mNewDirIconPath = path;
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

    void ContextManagerWidget::handleFocusChanged(QWidget* oldWidget, QWidget* newWidget)
    {
        if(!newWidget) return;
        if(newWidget->parent() == this)
        {
            mShortCutDeleteItem->setEnabled(true);
            return;
        }
        else
        {
            mShortCutDeleteItem->setEnabled(false);
            return;
        }
    }

}
