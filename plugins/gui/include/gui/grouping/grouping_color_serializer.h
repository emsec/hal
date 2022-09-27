//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/netlist/project_serializer.h"
#include <QModelIndex>

namespace hal {
    class GroupingTableModel;
    class ModuleModel;

    class GroupingColorSerializer : public ProjectSerializer
    {
        void restoreGroupingColor(const std::filesystem::path& loaddir, const std::string& jsonfile, GroupingTableModel *gtm = nullptr);
    public:
        GroupingColorSerializer();

        std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) override;

        void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) override;

        void restore(GroupingTableModel *gtm);
    };

    class ModuleColorSerializer : public ProjectSerializer
    {
        void restoreModuleColor(const std::filesystem::path& loaddir, const std::string& jsonfile, ModuleModel *mm = nullptr);
        void serializeColorRecursion(QJsonArray& mcArr, const ModuleModel* mm, QModelIndex parent=QModelIndex());
    public:
        ModuleColorSerializer();

        std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) override;

        void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) override;

        void restore(ModuleModel *mm);
    };
}
