#pragma once

#include "gui/gui_utils/sort.h"

#include <QSortFilterProxyModel>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Enables filtering of nets or gates in the ModuleModel.
     *
     */
    class FilterElementsProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        FilterElementsProxyModel(QObject* parent = nullptr);

        /**
         * Sets whether or not nets are filtered out by the filter.
         * @returns <b>true</b> if nets are filtered out now. <b>false</b> if not. 
         * @param filterNets if <b>true</b>, then nets are filtered out.
         */
        void setFilterNets(bool filterNets);

        /**
         * Sets whether or not gates are filtered out by the filter.
         * @returns <b>true</b> if gates are filtered out now. <b>false</b> if not. 
         * @param filterNets if <b>true</b>, then gates are filtered out.
         */
        void setFilterGates(bool filterGates);

        /**
         * Returns whether or not nets are filtered out by the filter.
        */
        bool areNetsFiltered();

        /**
         * Returns whether or not gates are filtered out by the filter.
        */
        bool areGatesFiltered();

    protected:
        /**
         * Overrides QSortFilterProxyModel::filterAcceptsRow to implement the filter logic. 
         *
         * @param sourceRow - The row in the source model
         * @param sourceParent - The source parent
         * @returns <b>true</b> if the row should be included in the model.
         */
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    private:
        bool mFilterNets = false;
        bool mFilterGates = false;
    };
}
