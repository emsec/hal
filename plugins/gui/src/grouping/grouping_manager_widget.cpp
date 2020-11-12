#include "gui/grouping/grouping_manager_widget.h"

#include "gui/gui_globals.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/grouping/grouping_color_delegate.h"
#include "gui/grouping/grouping_proxy_model.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"

#include "hal_core/utilities/log.h"

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
          mProxyModel(new GroupingProxyModel(this)),
          mSearchbar(new Searchbar(this)),
          mNewGroupingAction(new QAction(this)),
          mRenameAction(new QAction(this)),
          mColorSelectAction(new QAction(this)),
          mDeleteAction(new QAction(this)),
          mToSelectionAction(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();
        mTabView = tab_view;

        mNewGroupingAction->setIcon(gui_utility::getStyledSvgIcon(mNewGroupingIconStyle, mNewGroupingIconPath));
        mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mRenameGroupingIconStyle, mRenameGroupingIconPath));
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(mDeleteIconStyle, mDeleteIconPath));
        mColorSelectAction->setIcon(gui_utility::getStyledSvgIcon(mColorSelectIconStyle, mColorSelectIconPath));
        mToSelectionAction->setIcon(gui_utility::getStyledSvgIcon(mToSelectionIconStyle, mToSelectionIconPath));

        mNewGroupingAction->setToolTip("New");
        mRenameAction->setToolTip("Rename");
        mColorSelectAction->setToolTip("Color");
        mDeleteAction->setToolTip("Delete");
        mToSelectionAction->setToolTip("To selection");

        mNewGroupingAction->setText("Create new grouping");
        mRenameAction->setText("Rename grouping");
        mColorSelectAction->setText("Select color for grouping");
        mDeleteAction->setText("Delete grouping");
        mToSelectionAction->setText("Add grouping to selection");

        //mOpenAction->setEnabled(false);
        //mRenameAction->setEnabled(false);
        //mDeleteAction->setEnabled(false);

        mGroupingTableModel = new GroupingTableModel;

        mProxyModel->setSourceModel(mGroupingTableModel);
        mProxyModel->setSortRole(Qt::UserRole);

        mGroupingTableView = new QTableView(this);
        mGroupingTableView->setModel(mProxyModel);
        mGroupingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGroupingTableView->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        mGroupingTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        mGroupingTableView->verticalHeader()->hide();
        mGroupingTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mGroupingTableView->setItemDelegateForColumn(2,new GroupingColorDelegate(mGroupingTableView));
        mGroupingTableView->setSortingEnabled(true);
        mGroupingTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);

        mGroupingTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);
        QFont font = mGroupingTableView->horizontalHeader()->font();
        font.setBold(true);
        mGroupingTableView->horizontalHeader()->setFont(font);
        mContentLayout->addWidget(mGroupingTableView);
        mContentLayout->addWidget(mSearchbar);

        mSearchbar->hide();

        connect(mSearchbar, &Searchbar::textEdited, this, &GroupingManagerWidget::filter);

        connect(mNewGroupingAction, &QAction::triggered, this, &GroupingManagerWidget::handleCreateGroupingClicked);
        connect(mRenameAction, &QAction::triggered, this, &GroupingManagerWidget::handleRenameGroupingClicked);
        connect(mColorSelectAction, &QAction::triggered, this, &GroupingManagerWidget::handleColorSelectClicked);
        connect(mToSelectionAction, &QAction::triggered, this, &GroupingManagerWidget::handleToSelectionClicked);
        connect(mDeleteAction, &QAction::triggered, this, &GroupingManagerWidget::handleDeleteGroupingClicked);

        connect(mGroupingTableView, &QTableView::customContextMenuRequested, this, &GroupingManagerWidget::handleContextMenuRequest);
        connect(mGroupingTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GroupingManagerWidget::handleCurrentChanged);
        connect(mGroupingTableModel, &GroupingTableModel::lastEntryDeleted, this, &GroupingManagerWidget::handleLastEntryDeleted);
        connect(mGroupingTableModel, &GroupingTableModel::newEntryAdded, this, &GroupingManagerWidget::handleNewEntryAdded);
        handleCurrentChanged();
    }

    QList<QShortcut*> GroupingManagerWidget::createShortcuts()
    {
        QShortcut* search_shortcut = gKeybindManager->makeShortcut(this, "keybinds/searchbar_toggle");
        connect(search_shortcut, &QShortcut::activated, this, &GroupingManagerWidget::toggleSearchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

        return list;
    }

    void GroupingManagerWidget::handleCreateGroupingClicked()
    {
        mGroupingTableModel->addDefaultEntry();
    }

    void GroupingManagerWidget::handleColorSelectClicked()
    {
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
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
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
        if (!currentIndex.isValid()) return;
        Grouping* grp = getCurrentGrouping().grouping();
        if (!grp) return;
        for (Module* m : grp->get_modules())
            gSelectionRelay->mSelectedModules.insert(m->get_id());
        for (Gate* g : grp->get_gates())
            gSelectionRelay->mSelectedGates.insert(g->get_id());
        for (Net* n : grp->get_nets())
            gSelectionRelay->mSelectedNets.insert(n->get_id());
        gSelectionRelay->relaySelectionChanged(this);
    }

    void GroupingManagerWidget::handleRenameGroupingClicked()
    {
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
        if (!currentIndex.isValid()) return;
        QModelIndex modelIndex = mGroupingTableModel->index(currentIndex.row(),0);

        InputDialog ipd;
        ipd.setWindowTitle("Rename Grouping");
        ipd.setInfoText("Please select a new unique name for the grouping.");
        QString oldName = mGroupingTableModel->data(modelIndex,Qt::DisplayRole).toString();
        mGroupingTableModel->setAboutToRename(oldName);
        ipd.setInputText(oldName);
        ipd.addValidator(mGroupingTableModel);

        if (ipd.exec() == QDialog::Accepted)
            mGroupingTableModel->renameGrouping(modelIndex.row(),ipd.textValue());
        mGroupingTableModel->setAboutToRename(QString());
    }

    void GroupingManagerWidget::handleDeleteGroupingClicked()
    {
        mGroupingTableModel->removeRows(mProxyModel->mapToSource(mGroupingTableView->currentIndex()).row());
    }

    void GroupingManagerWidget::handleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(deselected);

        if(selected.indexes().isEmpty())
            setToolbarButtonsEnabled(false);
        else
            setToolbarButtonsEnabled(true);
    }

    void GroupingManagerWidget::handleContextMenuRequest(const QPoint& point)
    {
        const QModelIndex clicked_index = mGroupingTableView->indexAt(point);

        QMenu context_menu;

        context_menu.addAction(mNewGroupingAction);

        if (clicked_index.isValid())
        {
            context_menu.addAction(mRenameAction);
            context_menu.addAction(mColorSelectAction);
            context_menu.addAction(mToSelectionAction);
            context_menu.addAction(mDeleteAction);
        }

        context_menu.exec(mGroupingTableView->viewport()->mapToGlobal(point));
    }

    GroupingTableEntry GroupingManagerWidget::getCurrentGrouping()
    {
        QModelIndex modelIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());

        return mGroupingTableModel->groupingAt(modelIndex.row());
    }

    void GroupingManagerWidget::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mNewGroupingAction);
        toolbar->addAction(mRenameAction);
        toolbar->addAction(mColorSelectAction);
        toolbar->addAction(mToSelectionAction);
        toolbar->addAction(mDeleteAction);
    }

    void GroupingManagerWidget::setToolbarButtonsEnabled(bool enabled)
    {
        mRenameAction->setEnabled(enabled);
        mColorSelectAction->setEnabled(enabled);
        mToSelectionAction->setEnabled(enabled);
        mDeleteAction->setEnabled(enabled);
    }

    void GroupingManagerWidget::handleNewEntryAdded(const QModelIndex& modelIndex)
    {
        if (!modelIndex.isValid()) return;
        QModelIndex proxyIndex = mProxyModel->mapFromSource(modelIndex);
        if (!proxyIndex.isValid()) return;
        mGroupingTableView->setCurrentIndex(proxyIndex);
        handleCurrentChanged(proxyIndex);
    }

    void GroupingManagerWidget::handleLastEntryDeleted()
    {
        if (mProxyModel->rowCount())
        {
            QModelIndex inx = mProxyModel->index(0,0);
            mGroupingTableView->setCurrentIndex(inx);
            handleCurrentChanged(inx);
        }
        else
            handleCurrentChanged();
    }

    void GroupingManagerWidget::handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        bool enable = mGroupingTableModel->rowCount() > 0 && current.isValid();
        QAction* entryBasedAction[] = { mRenameAction, mColorSelectAction,
                                        mDeleteAction, mToSelectionAction, nullptr};

        QStringList iconPath, iconStyle;
        iconPath << mRenameGroupingIconPath << mColorSelectIconPath
                             << mDeleteIconPath << mToSelectionIconPath;
        iconStyle << mRenameGroupingIconStyle << mColorSelectIconStyle
                              << mDeleteIconStyle << mToSelectionIconStyle;

        for (int iacc = 0; entryBasedAction[iacc]; iacc++)
        {
            entryBasedAction[iacc]->setEnabled(enable);
            entryBasedAction[iacc]->setIcon(
                        gui_utility::getStyledSvgIcon(enable
                                                         ? iconStyle.at(iacc)
                                                         : disabledIconStyle(),
                                                         iconPath.at(iacc)));
        }
    }

    void GroupingManagerWidget::toggleSearchbar()
    {
        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
            mSearchbar->hide();
    }

    void GroupingManagerWidget::filter(const QString& text)
    {
        QRegExp* regex = new QRegExp(text);
        if (regex->isValid())
        {
            mProxyModel->setFilterRegExp(*regex);
            QString output = "Groupings widget regular expression '" + text + "' entered.";
            log_info("user", output.toStdString());
        }
    }

    QString GroupingManagerWidget::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    QString GroupingManagerWidget::newGroupingIconPath() const
    {
        return mNewGroupingIconPath;
    }

    QString GroupingManagerWidget::newGroupingIconStyle() const
    {
        return mNewGroupingIconStyle;
    }

    QString GroupingManagerWidget::renameGroupingIconPath() const
    {
        return mRenameGroupingIconPath;
    }

    QString GroupingManagerWidget::renameGroupingIconStyle() const
    {
        return mRenameGroupingIconStyle;
    }

    QString GroupingManagerWidget::deleteIconPath() const
    {
        return mDeleteIconPath;
    }

    QString GroupingManagerWidget::deleteIconStyle() const
    {
        return mDeleteIconStyle;
    }

    QString GroupingManagerWidget::colorSelectIconPath() const
    {
        return mColorSelectIconPath;
    }

    QString GroupingManagerWidget::colorSelectIconStyle() const
    {
        return mColorSelectIconStyle;
    }

    QString GroupingManagerWidget::toSelectionIconPath() const
    {
        return mToSelectionIconPath;
    }

    QString GroupingManagerWidget::toSelectionIconStyle() const
    {
        return mToSelectionIconStyle;
    }

    void GroupingManagerWidget::setDisabledIconStyle(const QString& style)
    {
        mDisabledIconStyle = style;
    }

    void GroupingManagerWidget::setNewGroupingIconPath(const QString& path)
    {
        mNewGroupingIconPath = path;
    }

    void GroupingManagerWidget::setNewGroupingIconStyle(const QString& style)
    {
        mNewGroupingIconStyle = style;
    }

    void GroupingManagerWidget::setRenameGroupingIconPath(const QString& path)
    {
        mRenameGroupingIconPath = path;
    }

    void GroupingManagerWidget::setRenameGroupingIconStyle(const QString& style)
    {
        mRenameGroupingIconStyle = style;
    }

    void GroupingManagerWidget::setDeleteIconPath(const QString& path)
    {
        mDeleteIconPath = path;
    }

    void GroupingManagerWidget::setDeleteIconStyle(const QString& style)
    {
        mDeleteIconStyle = style;
    }

    void GroupingManagerWidget::setColorSelectIconPath(const QString& path)
    {
        mColorSelectIconPath = path;
    }

    void GroupingManagerWidget::setColorSelectIconStyle(const QString& style)
    {
        mColorSelectIconStyle = style;
    }

    void GroupingManagerWidget::setToSelectionIconPath(const QString& path)
    {
        mToSelectionIconPath = path;
    }

    void GroupingManagerWidget::setToSelectionIconStyle(const QString& style)
    {
        mToSelectionIconStyle = style;
    }
}
