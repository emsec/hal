#include "gui/grouping/grouping_manager_widget.h"

#include "gui/action/action.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/grouping/grouping_color_delegate.h"
#include "gui/grouping/grouping_proxy_model.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "gui/toolbar/toolbar.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_color.h"
#include "gui/user_action/user_action_compound.h"
#include "hal_core/utilities/log.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QHeaderView>
#include <QImage>
#include <QMenu>
#include <QResizeEvent>
#include <QShortcut>
#include <QSize>
#include <QStringList>
#include <QVBoxLayout>
#include <unordered_set>

namespace hal
{
    GroupingManagerWidget::GroupingManagerWidget(QWidget* parent)
        : ContentWidget("Groupings", parent), mProxyModel(new GroupingProxyModel(this)), mSearchbar(new Searchbar(this)), mNewGroupingAction(new QAction(this)),
          mRenameAction(new QAction(this)), mColorSelectAction(new QAction(this)), mDeleteAction(new QAction(this)), mToSelectionAction(new QAction(this)), mTableAction(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();

        mNewGroupingAction->setIcon(gui_utility::getStyledSvgIcon(mNewGroupingIconStyle, mNewGroupingIconPath));
        mRenameAction->setIcon(gui_utility::getStyledSvgIcon(mRenameGroupingIconStyle, mRenameGroupingIconPath));
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(mDeleteIconStyle, mDeleteIconPath));
        mColorSelectAction->setIcon(gui_utility::getStyledSvgIcon(mColorSelectIconStyle, mColorSelectIconPath));
        mToSelectionAction->setIcon(gui_utility::getStyledSvgIcon(mToSelectionIconStyle, mToSelectionIconPath));
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
        mTableAction->setIcon(gui_utility::getStyledSvgIcon(mTableIconStyle, mTableIconPath));

        mNewGroupingAction->setToolTip("New");
        mRenameAction->setToolTip("Rename");
        mColorSelectAction->setToolTip("Color");
        mDeleteAction->setToolTip("Delete");
        mToSelectionAction->setToolTip("To selection");
        mSearchAction->setToolTip("Search");
        mTableAction->setToolTip("Show content");

        mNewGroupingAction->setText("Create new grouping");
        mRenameAction->setText("Rename grouping");
        mColorSelectAction->setText("Select color for grouping");
        mDeleteAction->setText("Delete grouping");
        mToSelectionAction->setText("Add grouping to selection");
        mSearchAction->setText("Search");
        mTableAction->setText("Show content");

        //mOpenAction->setEnabled(false);
        //mRenameAction->setEnabled(false);
        //mDeleteAction->setEnabled(false);

        mGroupingTableModel = new GroupingTableModel(false, this);
        mColorSerializer.restore(mGroupingTableModel);

        mProxyModel->setSourceModel(mGroupingTableModel);
        mProxyModel->setSortRole(Qt::UserRole);

        mGroupingTableView = new QTableView(this);
        mGroupingTableView->setModel(mProxyModel);
        mGroupingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGroupingTableView->setSelectionMode(QAbstractItemView::SingleSelection);    // ERROR ???
        mGroupingTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        mGroupingTableView->verticalHeader()->hide();
        mGroupingTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mGroupingTableView->setItemDelegateForColumn(2, new GroupingColorDelegate(mGroupingTableView));
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
        connect(mSearchbar, &Searchbar::textEdited, this, &GroupingManagerWidget::updateSearchIcon);

        connect(mNewGroupingAction, &QAction::triggered, this, &GroupingManagerWidget::handleCreateGroupingClicked);
        connect(mRenameAction, &QAction::triggered, this, &GroupingManagerWidget::handleRenameGroupingClicked);
        connect(mColorSelectAction, &QAction::triggered, this, &GroupingManagerWidget::handleColorSelectClicked);
        connect(mToSelectionAction, &QAction::triggered, this, &GroupingManagerWidget::handleToSelectionClicked);
        connect(mDeleteAction, &QAction::triggered, this, &GroupingManagerWidget::handleDeleteGroupingClicked);
        connect(mSearchAction, &QAction::triggered, this, &GroupingManagerWidget::toggleSearchbar);
        connect(mTableAction, &QAction::triggered, this, &GroupingManagerWidget::handleShowContentClicked);

        connect(mGroupingTableView, &QTableView::customContextMenuRequested, this, &GroupingManagerWidget::handleContextMenuRequest);
        connect(mGroupingTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GroupingManagerWidget::handleCurrentChanged);
        connect(mGroupingTableModel, &GroupingTableModel::lastEntryDeleted, this, &GroupingManagerWidget::handleLastEntryDeleted);
        connect(mGroupingTableModel, &GroupingTableModel::newEntryAdded, this, &GroupingManagerWidget::handleNewEntryAdded);
        connect(mGroupingTableView, &QTableView::doubleClicked, this, &GroupingManagerWidget::handleDoubleClicked);
        handleCurrentChanged();

        mShortCutDeleteItem = new QShortcut(ContentManager::sSettingDeleteItem->value().toString(), this);
        mShortCutDeleteItem->setEnabled(false);

        connect(ContentManager::sSettingDeleteItem, &SettingsItemKeybind::keySequenceChanged, mShortCutDeleteItem, &QShortcut::setKey);
        connect(mShortCutDeleteItem, &QShortcut::activated, this, &GroupingManagerWidget::handleDeleteGroupingClicked);

        connect(qApp, &QApplication::focusChanged, this, &GroupingManagerWidget::handleDeleteShortcutOnFocusChanged);
    }

    QList<QShortcut*> GroupingManagerWidget::createShortcuts()
    {
        mSearchShortcut = new QShortcut(mSearchKeysequence, this);
        connect(mSearchShortcut, &QShortcut::activated, mSearchAction, &QAction::trigger);

        QList<QShortcut*> list;
        list.append(mSearchShortcut);

        return list;
    }

    QIcon GroupingManagerWidget::toolboxIcon() const
    {
        QIcon retval(":/icons/toolbox");
        QImage img(":/icons/toolbox");
        for (int x = 0; x < img.width(); x++)
            for (int y = 0; y < img.width(); y++)
            {
                int h, s, v;
                QColor col = img.pixelColor(x, y);
                col.getHsv(&h, &s, &v);
                v /= 3;
                col.setHsv(h, s, v);
                img.setPixelColor(x, y, col);
            }
        retval.addPixmap(QPixmap::fromImage(img), QIcon::Disabled);
        return retval;
    }

    void GroupingManagerWidget::handleCreateGroupingClicked()
    {
        ActionCreateObject* act = new ActionCreateObject(UserActionObjectType::Grouping);
        act->exec();
    }

    void GroupingManagerWidget::handleColorSelectClicked()
    {
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
        if (!currentIndex.isValid())
            return;
        QModelIndex nameIndex  = mGroupingTableModel->index(currentIndex.row(), 0);
        QString name           = mGroupingTableModel->data(nameIndex, Qt::DisplayRole).toString();
        QModelIndex idIndex    = mGroupingTableModel->index(currentIndex.row(), 1);
        u32 id                 = mGroupingTableModel->data(idIndex, Qt::DisplayRole).toInt();
        QModelIndex modelIndex = mGroupingTableModel->index(currentIndex.row(), 2);
        QColor color           = mGroupingTableModel->data(modelIndex, Qt::BackgroundRole).value<QColor>();
        color                  = QColorDialog::getColor(color, this, "Select color for grouping " + name);
        if (color.isValid())
        {
            ActionSetObjectColor* act = new ActionSetObjectColor(color);
            act->setObject(UserActionObject(id, UserActionObjectType::Grouping));
            act->exec();
        }
    }

    void GroupingManagerWidget::handleToolboxClicked()
    {
        QMenu* toolboxMenu       = new QMenu(this);
        QAction* toolPredecessor = new Action("Predecessor tree to new grouping");
        connect(toolPredecessor, &QAction::triggered, this, &GroupingManagerWidget::handleToolboxPredecessor);
        toolboxMenu->addAction(toolPredecessor);
        QAction* toolSuccessor = new Action("Successor tree to new grouping");
        connect(toolSuccessor, &QAction::triggered, this, &GroupingManagerWidget::handleToolboxSuccessor);
        toolboxMenu->addAction(toolSuccessor);
        QAction* toolPreStep = new Action("Groupings by predecessor distance");
        toolboxMenu->addAction(toolPreStep);
        connect(toolPreStep, &QAction::triggered, this, &GroupingManagerWidget::handleToolboxPredecessorDistance);
        QAction* toolSucStep = new Action("Groupings by successor distance");
        toolboxMenu->addAction(toolSucStep);
        connect(toolSucStep, &QAction::triggered, this, &GroupingManagerWidget::handleToolboxSuccessorDistance);
        toolboxMenu->exec(mapToGlobal(pos() + QPoint(100, -25)));
    }

    void GroupingManagerWidget::handleDoubleClicked(const QModelIndex& index)
    {
        if (index.column() == 0)
            handleShowContentClicked();
        if (index.column() == 2)
            handleColorSelectClicked();
    }

    GroupingManagerWidget::ToolboxNode::ToolboxNode(Endpoint* ep, const ToolboxModuleHash* tmh)
    {
        if (ep)
        {
            Gate* g = ep->get_gate();
            if (tmh && tmh->mHash.contains(g))
                mNode = Node(tmh->mHash.value(g)->get_id(), Node::Module);
            else
                mNode = Node(g->get_id(), Node::Gate);
        }
        else
        {
            if (gSelectionRelay->numberSelectedModules())
                mNode = Node(gSelectionRelay->selectedModulesList().at(0), Node::Module);
            else
                mNode = Node(gSelectionRelay->selectedGatesList().at(0), Node::Gate);
        }
        switch (mNode.type())
        {
            case Node::Module:
                mName = QString::fromStdString(gNetlist->get_module_by_id(mNode.id())->get_name());
                break;
            case Node::Gate:
                mName = QString::fromStdString(gNetlist->get_gate_by_id(mNode.id())->get_name());
                break;
            default:
                return;
        }
    }

    GroupingManagerWidget::ToolboxNode::ToolboxNode(const GraphicsItem* item)
    {
        switch (item->itemType())
        {
            case ItemType::Module:
                mNode = Node(item->id(), Node::Module);
                mName = QString::fromStdString(gNetlist->get_module_by_id(mNode.id())->get_name());
                break;
            case ItemType::Gate:
                mNode = Node(item->id(), Node::Gate);
                mName = QString::fromStdString(gNetlist->get_gate_by_id(mNode.id())->get_name());
                break;
            default:
                break;
        }
    }

    std::vector<Net*> GroupingManagerWidget::ToolboxNode::inputNets() const
    {
        switch (mNode.type())
        {
            case Node::Module:
                return utils::to_vector(gNetlist->get_module_by_id(mNode.id())->get_input_nets());
            case Node::Gate:
                return gNetlist->get_gate_by_id(mNode.id())->get_fan_in_nets();
            default:
                break;
        }
        return std::vector<Net*>();
    }

    std::vector<Net*> GroupingManagerWidget::ToolboxNode::outputNets() const
    {
        switch (mNode.type())
        {
            case Node::Module:
                return utils::to_vector(gNetlist->get_module_by_id(mNode.id())->get_output_nets());
            case Node::Gate:
                return gNetlist->get_gate_by_id(mNode.id())->get_fan_out_nets();
            default:
                break;
        }
        return std::vector<Net*>();
    }

    GroupingManagerWidget::ToolboxModuleHash::ToolboxModuleHash(const Node& nd)
    {
        Module* parentModule = nullptr;
        switch (nd.type())
        {
            case Node::Module:
                parentModule = gNetlist->get_module_by_id(nd.id())->get_parent_module();
                break;
            case Node::Gate:
                parentModule = gNetlist->get_gate_by_id(nd.id())->get_module();
            default:
                break;
        }
        if (!parentModule)
            return;
        for (Module* sm : parentModule->get_submodules())
            for (Gate* g : sm->get_gates(nullptr, true))
                mHash[g] = sm;
    }

    void GroupingManagerWidget::handleToolboxPredecessor()
    {
        newGroupingSuccOrPred(0, false, nullptr);
    }

    void GroupingManagerWidget::handleToolboxSuccessor()
    {
        newGroupingSuccOrPred(0, true, nullptr);
    }

    void GroupingManagerWidget::newGroupingSuccOrPred(int maxDepth, bool succ, const GraphicsItem* item)
    {
        QSet<u32> mods, gats, nets;
        ToolboxNode tbn;
        if (item)
            tbn = ToolboxNode(item);
        ToolboxModuleHash tmh(tbn.mNode);
        QVector<Net*> todoNet;
        QSet<Node> handledBox;
        QSet<Net*> handledNet;
        todoNet.append(QVector<Net*>::fromStdVector(succ ? tbn.outputNets() : tbn.inputNets()));
        handledBox.insert(tbn.mNode);
        for (int loop = 0; !maxDepth || loop < maxDepth; loop++)
        {
            if (todoNet.isEmpty())
                break;
            QVector<Net*> nextRound;
            for (Net* n : todoNet)
            {
                if (handledNet.contains(n))
                    continue;
                handledNet.insert(n);
                nets.insert(n->get_id());
                for (Endpoint* ep : (succ ? n->get_destinations() : n->get_sources()))
                {
                    ToolboxNode nextNode(ep, &tmh);
                    if (handledBox.contains(nextNode.mNode))
                        continue;
                    handledBox.insert(nextNode.mNode);
                    switch (nextNode.mNode.type())
                    {
                        case Node::Module:
                            mods.insert(nextNode.mNode.id());
                            break;
                        case Node::Gate:
                            gats.insert(nextNode.mNode.id());
                            break;
                        default:
                            continue;
                    }
                    nextRound.append(QVector<Net*>::fromStdVector(succ ? nextNode.outputNets() : nextNode.inputNets()));
                }
            }
            todoNet = nextRound;
        }

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::Grouping, tbn.mName + (succ ? " successor" : " predecessor")));
        act->addAction(new ActionAddItemsToObject(mods, gats, nets));
        act->exec();
    }

    void GroupingManagerWidget::handleToolboxPredecessorDistance()
    {
        newGroupingByDistance(3, false, nullptr);
    }

    void GroupingManagerWidget::handleToolboxSuccessorDistance()
    {
        newGroupingByDistance(3, true, nullptr);
    }

    void GroupingManagerWidget::newGroupingByDistance(int maxDepth, bool succ, const GraphicsItem* item)
    {
        ToolboxNode tbn;
        if (item)
            tbn = ToolboxNode(item);
        ToolboxModuleHash tmh(tbn.mNode);
        QVector<Net*> thisDst;
        QVector<Net*> nextDst;
        QSet<Node> handledNode;
        QSet<Net*> handledNet;
        nextDst.append(QVector<Net*>::fromStdVector(succ ? tbn.outputNets() : tbn.inputNets()));
        handledNode.insert(tbn.mNode);
        for (int iround = 1; iround <= maxDepth; iround++)
        {
            if (nextDst.isEmpty())
                break;
            thisDst = nextDst;
            nextDst.clear();
            QSet<u32> mods, gats, nets;
            for (Net* n : thisDst)
            {
                if (handledNet.contains(n))
                    continue;
                handledNet.insert(n);
                nets.insert(n->get_id());
                for (Endpoint* ep : succ ? n->get_destinations() : n->get_sources())
                {
                    ToolboxNode nextNode(ep, &tmh);
                    if (handledNode.contains(nextNode.mNode))
                        continue;
                    handledNode.insert(nextNode.mNode);
                    switch (nextNode.mNode.type())
                    {
                        case Node::Module:
                            mods.insert(nextNode.mNode.id());
                            break;
                        case Node::Gate:
                            gats.insert(nextNode.mNode.id());
                            break;
                        default:
                            continue;
                    }
                    nextDst.append(QVector<Net*>::fromStdVector(succ ? nextNode.outputNets() : nextNode.inputNets()));
                }
            }
            if (!mods.isEmpty() || !nets.isEmpty() || !gats.isEmpty())
            {
                UserActionCompound* act = new UserActionCompound;
                act->setUseCreatedObject();
                act->addAction(new ActionCreateObject(UserActionObjectType::Grouping, QString("%1 step to %2").arg(iround).arg(tbn.mName)));
                act->addAction(new ActionAddItemsToObject(mods, gats, nets));
                act->exec();
            }
        }
    }

    void GroupingManagerWidget::handleToSelectionClicked()
    {
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
        if (!currentIndex.isValid())
            return;
        Grouping* grp = getCurrentGrouping().grouping();
        if (!grp)
            return;
        for (Module* m : grp->get_modules())
            gSelectionRelay->addModule(m->get_id());
        for (Gate* g : grp->get_gates())
            gSelectionRelay->addGate(g->get_id());
        for (Net* n : grp->get_nets())
            gSelectionRelay->addNet(n->get_id());
        gSelectionRelay->relaySelectionChanged(this);
    }

    void GroupingManagerWidget::handleRenameGroupingClicked()
    {
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
        if (!currentIndex.isValid())
            return;

        InputDialog ipd;
        ipd.setWindowTitle("Rename Grouping");
        ipd.setInfoText("Please select a new unique name for the grouping.");
        int irow        = currentIndex.row();
        QString oldName = mGroupingTableModel->data(mGroupingTableModel->index(irow, 0), Qt::DisplayRole).toString();
        mGroupingTableModel->setAboutToRename(oldName);
        ipd.setInputText(oldName);
        ipd.addValidator(mGroupingTableModel);

        if (ipd.exec() == QDialog::Accepted)
        {
            QString newName = ipd.textValue();
            if (newName != oldName)
            {
                ActionRenameObject* act = new ActionRenameObject(newName);
                u32 grpId               = mGroupingTableModel->data(mGroupingTableModel->index(irow, 1), Qt::DisplayRole).toInt();
                act->setObject(UserActionObject(grpId, UserActionObjectType::Grouping));
                act->exec();
            }
        }
        mGroupingTableModel->setAboutToRename(QString());
    }

    void GroupingManagerWidget::handleShowContentClicked()
    {
        QModelIndex currentIndex = mProxyModel->mapToSource(mGroupingTableView->currentIndex());
        if (!currentIndex.isValid())
            return;

        int irow = currentIndex.row();
        GroupingTableEntry gte = mGroupingTableModel->groupingAt(irow);

        QString grpName  = gte.name();
        u32     grpId    = gte.id();
        QColor  grpColor = gte.color();

        QDialog dialog;
        dialog.setWindowTitle(QString("Content of %1 (ID: %2)").arg(grpName).arg(grpId));


        // Create color rectangle
        QLabel* colorRectangle = new QLabel();
        colorRectangle->setText(QString());  // Empty text to visualize grouping color
        colorRectangle->setStyleSheet("background-color: " + grpColor.name());
        colorRectangle->setFixedSize(25, 25);  // Adapt size of grouping color
        colorRectangle->setAutoFillBackground(true);

        // Replace InputDialog with SelectionTreeView
        SelectionTreeView* selectionTreeView = new SelectionTreeView(&dialog, true);
        selectionTreeView->populate(true, grpId);

        QPushButton* closeButton = new QPushButton("Close", &dialog);
        connect(closeButton, &QPushButton::clicked, [&dialog](){ dialog.close(); });

        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        QHBoxLayout* hlay = new QHBoxLayout;
        hlay->addStretch();
        hlay->addWidget(colorRectangle);
        layout->addLayout(hlay);
        layout->addWidget(selectionTreeView);
        layout->addWidget(closeButton);

        dialog.exec();
    }

    void GroupingManagerWidget::handleDeleteGroupingClicked()
    {
        if (!hasFocus() && !mGroupingTableView->hasFocus()) return;

        QModelIndex current     = mGroupingTableView->currentIndex();
        if (!current.isValid()) return;
        int irow                = mProxyModel->mapToSource(mGroupingTableView->currentIndex()).row();
        u32 grpId               = mGroupingTableModel->groupingAt(irow).id();
        ActionDeleteObject* act = new ActionDeleteObject;
        act->setObject(UserActionObject(grpId, UserActionObjectType::Grouping));
        act->exec();
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
            context_menu.addAction(mTableAction);
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
        toolbar->addAction(mSearchAction);
        toolbar->addAction(mTableAction);
        mSearchAction->setEnabled(mGroupingTableModel->rowCount() > 0);
    }

    void GroupingManagerWidget::setToolbarButtonsEnabled(bool enable)
    {
        mRenameAction->setEnabled(enable);
        mColorSelectAction->setEnabled(enable);
        mToSelectionAction->setEnabled(enable);
        mDeleteAction->setEnabled(enable);
        mTableAction->setEnabled(enable);
    }

    void GroupingManagerWidget::handleNewEntryAdded(const QModelIndex& modelIndexName)
    {
        if (!modelIndexName.isValid())
            return;
        QModelIndex proxyIndex = mProxyModel->mapFromSource(modelIndexName);
        if (!proxyIndex.isValid())
            return;
        mGroupingTableView->setCurrentIndex(proxyIndex);
        handleCurrentChanged(proxyIndex);
    }

    void GroupingManagerWidget::handleLastEntryDeleted()
    {
        if (mProxyModel->rowCount())
        {
            QModelIndex inx = mProxyModel->index(0, 0);
            mGroupingTableView->setCurrentIndex(inx);
            handleCurrentChanged(inx);
        }
        else
            handleCurrentChanged();
    }

    void GroupingManagerWidget::handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        bool enable                 = mGroupingTableModel->rowCount() > 0 && current.isValid();
        QAction* entryBasedAction[] = {mRenameAction, mColorSelectAction, mDeleteAction, mToSelectionAction, mTableAction, nullptr};

        QStringList iconPath, iconStyle;
        iconPath << mRenameGroupingIconPath << mColorSelectIconPath << mDeleteIconPath << mToSelectionIconPath << mTableIconPath;
        iconStyle << mRenameGroupingIconStyle << mColorSelectIconStyle << mDeleteIconStyle << mToSelectionIconStyle << mTableIconStyle;

        for (int iacc = 0; entryBasedAction[iacc]; iacc++)
        {
            entryBasedAction[iacc]->setEnabled(enable);
            entryBasedAction[iacc]->setIcon(gui_utility::getStyledSvgIcon(enable ? iconStyle.at(iacc) : disabledIconStyle(), iconPath.at(iacc)));
        }
        enableSearchbar(mGroupingTableModel->rowCount() > 0);
    }

    void GroupingManagerWidget::toggleSearchbar()
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

    void GroupingManagerWidget::filter(const QString& text)
    {
        QRegularExpression* regex = new QRegularExpression(text);
        if (regex->isValid())
        {
            mProxyModel->setFilterRegularExpression(*regex);
            QString output = "Groupings widget regular expression '" + text + "' entered.";
            log_info("user", output.toStdString());
        }
    }

    void GroupingManagerWidget::updateSearchIcon()
    {
        if (mSearchbar->filterApplied() && mSearchbar->isVisible())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    void GroupingManagerWidget::enableSearchbar(bool enable)
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

    QString GroupingManagerWidget::toolboxIconPath() const
    {
        return mToolboxIconPath;
    }

    QString GroupingManagerWidget::toolboxIconStyle() const
    {
        return mToolboxIconStyle;
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

    void GroupingManagerWidget::setToolboxIconPath(const QString& path)
    {
        mToolboxIconPath = path;
    }

    void GroupingManagerWidget::setToolboxIconStyle(const QString& style)
    {
        mToolboxIconStyle = style;
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

    QString GroupingManagerWidget::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString GroupingManagerWidget::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString GroupingManagerWidget::searchActiveIconStyle() const
    {
        return mSearchActiveIconStyle;
    }

    QString GroupingManagerWidget::tableIconPath() const
    {
        return mTableIconPath;
    }

    QString GroupingManagerWidget::tableIconStyle() const
    {
        return mTableIconStyle;
    }

    void GroupingManagerWidget::setSearchIconPath(const QString& path)
    {
        mSearchIconPath = path;
    }

    void GroupingManagerWidget::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }

    void GroupingManagerWidget::setSearchActiveIconStyle(const QString& style)
    {
        mSearchActiveIconStyle = style;
    }

    void GroupingManagerWidget::setTableIconPath(const QString& path)
    {
        mTableIconPath = path;
    }

    void GroupingManagerWidget::setTableIconStyle(const QString& style)
    {
        mTableIconStyle = style;
    }

    void GroupingManagerWidget::handleDeleteShortcutOnFocusChanged(QWidget* oldWidget, QWidget* newWidget)
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
}    // namespace hal
