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
#include <QAbstractTableModel>
#include <QList>
#include <QComboBox>

namespace hal {
    class Net;

    class LogicEvaluatorTruthtableColumn
    {
        int mRows;
        int* mArray;
    public:
        LogicEvaluatorTruthtableColumn(int nrows, QList<int> values);
        ~LogicEvaluatorTruthtableColumn();
        int data(int irow) const;
        bool lessThan(const LogicEvaluatorTruthtableColumn& other, const QList<int>& sortRows) const;
    };

    class LogicEvaluatorTruthtableModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        enum DisplayFormat{ ZeroOne, LowHigh, BlueRed, MAXFORMAT };
    private:
        DisplayFormat mDisplayFormat;
        QList<const Net*> mInputList;
        QList<const Net*> mOutputList;
        QList<LogicEvaluatorTruthtableColumn*> mColumnList;
        int mInputSize;
        int mOutputSize;
    public Q_SLOTS:
        void sortModelRow(int irow);
        void sortModelRows(const QList<int>& sortRows);
    public:
        LogicEvaluatorTruthtableModel(const QList<const Net*>& inpList, const QList<const Net*>& outList, QObject* parent = nullptr);
        ~LogicEvaluatorTruthtableModel();
        QList<const Net*> getNets() const { return mInputList + mOutputList; }
        QMap<const Net*, int> selectedColumn(int icol) const;

        void setDisplayFormat(DisplayFormat df);
        void addColumn(LogicEvaluatorTruthtableColumn* letc);
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    };

    class LogicEvaluatorTruthtableSort : public QDialog
    {
        Q_OBJECT
        QComboBox* mSortKey[5];
        QList<const Net*> mNets;
    public:
        LogicEvaluatorTruthtableSort(QList<const Net*>& nets, QWidget* parent = nullptr);
        QList<int> sortOrder() const;
    };

    class LogicEvaluatorTruthtable : public QDialog
    {
        Q_OBJECT
        QAction* mActionDisplayFormat[LogicEvaluatorTruthtableModel::MAXFORMAT];
        QAction* mActionSort;
        LogicEvaluatorTruthtableModel* mModel;
        int mColumnDubbleClicked;
    private Q_SLOT:
        void handleDisplayFormatChanged(QAction* act);
        void handleSortTriggered();
        void handleColumnDubbleClicked(int icol);
    public:
        LogicEvaluatorTruthtable(LogicEvaluatorTruthtableModel* model, QWidget* parent = nullptr);
        QMap<const Net*, int> selectedColumn() const;
    };
}
