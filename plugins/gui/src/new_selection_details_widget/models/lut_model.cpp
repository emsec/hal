
#include "gui/new_selection_details_widget/models/lut_model.h"

namespace hal
{

    LutModel::LutModel(QObject* parent){
        // TODO
    }

    int LutModel::columnCount(const QModelIndex &parent) const
    {
        // TODO
        return 1;
    }

    int LutModel::rowCount(const QModelIndex &parent) const
    {
        // TODO
        return 1;
    }

    QVariant LutModel::data(const QModelIndex &index, int role) const
    {
        // TODO
        return QVariant();
    }

    QVariant LutModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        // TODO
        return QVariant();
    }

    bool LutModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        // TODO
        return false;
    }

} // namespace hal


