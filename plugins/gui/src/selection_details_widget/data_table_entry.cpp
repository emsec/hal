#include "gui/include/gui/selection_details_widget/data_table_entry.h"
#include "hal_core/netlist/data_container.h"

namespace hal {
    DataTableEntry::DataTableEntry(const QString& c, const QString& k, const QString t, const QString v)
        : category(c), key(k), dataType(t), value(v)
    {;}

    QString DataTableEntry::getPropertyValueByPropType(PropertyType ptype) const
    {
        switch (ptype) {
            case CategoryProperty: return category;
            case KeyProperty:      return key;
            case TypeProperty:     return dataType;
            case ValueProperty:    return value;
        }

        return QString();
    }

    bool DataTableEntry::isNull() const
    {
        return category.isEmpty() && key.isEmpty();
    }

    DataTableEntry DataTableEntry::fromContainer(DataContainer* container, std::string category, std::string key)
    {
        if (!container->has_data(category, key)) return DataTableEntry();

        std::tuple<std::string,std::string> res = container->get_data(category,key);

        return DataTableEntry({QString::fromStdString(category), QString::fromStdString(key),
                               QString::fromStdString(std::get<0>(res)), QString::fromStdString(std::get<1>(res))});
    }

    void DataTableEntry::toContainer(DataContainer* container) const
    {
        container->set_data(category.toStdString(), key.toStdString(), dataType.toStdString(), value.toStdString());
    }
}
