#pragma once

#include "hal_core/defines.h"
#include "module_select_model.h"
#include <QDialog>
#include <QAction>
#include <QKeySequence>
#include <QSet>

class QDialogButtonBox;
class QTreeView;
class QTabWidget;

namespace hal {

    class ModuleProxyModel;
    class Searchbar;

    class ModuleDialog : public QDialog
    {
        Q_OBJECT
    public:
        ModuleDialog(QWidget* parent=nullptr);
        u32 selectedId() const { return mSelectedId; }
        bool isNewModule() const { return mNewModule; }

    private Q_SLOTS:
        void handlePickFromGraph();
        void handleCreateNewModule();
        void handleToggleSearchbar();
        void handleTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void handleTreeDoubleClick(const QModelIndex& index);

    public Q_SLOTS:
        void handleTableSelection(u32 id, bool doubleClick);
        void accept() override;
        void keybindToggleSearchbar(const QKeySequence& seq);

    private:
        u32 mSelectedId;
        QDialogButtonBox* mButtonBox;
        ModuleSelectView* mTableView;
        QTreeView* mTreeView;
        ModuleSelectView* mLastUsed;
        QTabWidget* mTabWidget;

        ModuleProxyModel* mModuleTreeProxyModel;
        ModuleSelectModel* mModuleSelectModel;
        Searchbar* mSearchbar;
        QAction* mToggleSearchbar;
        ModuleSelectExclude mSelectExclude;

        bool mNewModule;

        void testOkEnabled();
        u32 treeModuleId(const QModelIndex& index);
    };
}

