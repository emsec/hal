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

#include "hal_core/defines.h"

#include <QAbstractTableModel>
#include <QList>

namespace hal
{

    class Net;

/**
 * @ingroup gui
 * @brief A model to display endpoint data.
 */
class EndpointTableModel : public QAbstractTableModel
{
    Q_OBJECT

private:
    struct Entry
    {
        QString name;
        u32 id;
        QString type;
        QString pin;
    };

public:
    enum Type
    {
        source,
        destination
    };

    /**
     * The constructor.
     *
     * @param parent - The model's parent.
     */
    EndpointTableModel(Type type, QObject* parent = nullptr);

    /**
     * Returns the amount of rows in the table.
     *
     * @param parent - The parent model index
     * @returns the amount of rows in the table.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns the amount of columns.
     *
     * @param parent - Then parent model index
     * @returns the amount of columns.
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns the data stored under the given role for a given index in the table model. <br>
     * All values are aligned centered.
     *
     * @param index - The index in the table
     * @param role - The access role
     * @returns the data
     */
    QVariant data(const QModelIndex& index, int role) const override;

    /**
     * Returns the header data fields
     *
     * @param section - The section (column) index
     * @param orientation - The orientation of the table
     * @param role - The access role
     * @returns the header data field at the given position
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * Clears the model.
     */
    void clear();

    /**
     * Sets the net this model will represent.
     *
     * @param net - The net.
     */
    void setNet(Net* net);

    /**
     * Get the ID of the net that is currently displayed.
     *
     * @return The net's id.
     */
    int getCurrentNetID();

    /**
     * Get the ID of the gate at the index. Index has to be valid.
     *
     * @param index - The index in the table
     * @return The gate's id.
     */
    u32 getGateIDFromIndex(const QModelIndex& index);

    /**
     * Get the gate's pin name from the index. Index has to be valid.
     *
     * @param index - The index.
     * @return The pin name.
     */
    QString getPinNameFromIndex(const QModelIndex& index);

    /**
     * Get the type that the endpoint-model represents
     * (source- or destination-endpoints of a net) as
     * a QString.
     *
     * @return The type.
     */
    QString typeString();

    /**
     * Get the type that the endpoint-model represents
     * (source- or destination-endpoints of a net).
     *
     * @return The type.
     */
    Type getType();

private:
    u32 mNetId;
    QList<Entry> mEntries;

    const Type mType;

};

}
