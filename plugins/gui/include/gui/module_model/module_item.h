//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "hal_core/defines.h"

#include <QColor>
#include <QList>
#include <QString>
#include <QVariant>

namespace hal
{
    class ModuleItem
    {
    public:
        ModuleItem(const u32 id);
        ModuleItem(const QString& name, const u32 id);

        void insertChild(int row, ModuleItem* child);
        void removeChild(ModuleItem* child);

        void appendChild(ModuleItem* child);
        void appendExistingChildIfAny(const QMap<u32,ModuleItem*>& moduleMap);
        void prependChild(ModuleItem* child);

        ModuleItem* parent();
        ModuleItem* child(int row);

        const ModuleItem* constParent() const;
        const ModuleItem* constChild(int row) const;

        int childCount() const;
        QVariant data(int column) const;
        int row() const;

        QString name() const;
        u32 id() const;
        QColor color() const;
        bool highlighted() const;

        void setParent(ModuleItem* parent);
        void set_name(const QString& name);
        void setColor(const QColor& color);
        void setHighlighted(const bool highlighted);

    private:
        ModuleItem* mParent;
        QList<ModuleItem*> mChildItems;

        u32 mId;
        QString mName;

        QColor mColor;
        bool mHighlighted;
    };
}
