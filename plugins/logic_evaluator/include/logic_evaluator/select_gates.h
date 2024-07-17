    // MIT License
    //
    // Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
    // Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
    // Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
    // Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
    //
    // Permission is hereby granted, free of charge, to any person obtaining a copy
    // of this software and associated documentation files (the "Software"), to deal
    // in the Software without restriction, including without limitation the rights
    // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    // copies of the Software, and to permit persons to whom the Software is
    // furnished to do so, subject to the following conditions:
    //
    // The above copyright notice and this permission notice shall be included in all
    // copies or substantial portions of the Software.
    //
    // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    // SOFTWARE.

    #pragma once

    #include <QDialog>
    #include <QTreeView>
    #include <vector>
    #include "gui/module_model/module_model.h"

    namespace hal {
        class Gate;

        class SelectGateItem : public ModuleItem
        {
            Qt::CheckState mState;
        public:
            SelectGateItem(u32 id, ModuleItem::TreeItemType type)
                : ModuleItem(id, type) {;}
            Qt::CheckState state() const { return mState; }
            void setState(Qt::CheckState stat) { mState = stat; }
        };

        class SelectGateModel : public ModuleModel
        {
            Q_OBJECT
            int insertModuleRecursion(const Module* mod, SelectGateItem* parentItem = nullptr);
            QPair<bool,bool> setCheckedRecursion(bool applySet, BaseTreeItem* parentItem, const QSet<u32>& selectedGateIds = QSet<u32>() );
            void setModuleStateRecursion(SelectGateItem* item, Qt::CheckState stat);
        public:
            SelectGateModel(QObject* parent = nullptr);
            void setChecked(const std::vector<Gate *> &gates);
            Qt::ItemFlags flags(const QModelIndex &index) const override;
            QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
            bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
        };

        class SelectGates : public QDialog
        {
            Q_OBJECT
            QTreeView* mTreeView;
        public:
            SelectGates(const std::vector<Gate *> &gates, QWidget* parent = nullptr);
        };
    }
