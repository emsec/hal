#include "gui/selection_details_widget/gate_details_widget/parameter_value_delegate.h"

#include "hal_core/utilities/enums.h"

#include <QComboBox>
#include <QLineEdit>

namespace hal
{
    ParameterValueDelegate::ParameterValueDelegate(QObject* parent) : QItemDelegate(parent)
    {
    }

    QWidget* ParameterValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        switch (parameterType(index))
        {
            case Parameter::Type::Boolean: {
                auto comboBox = new QComboBox(parent);
                comboBox->addItem("true");
                comboBox->addItem("false");
                comboBox->setFocusPolicy(Qt::StrongFocus);
                return comboBox;
            }
            case Parameter::Type::Integer: {
                auto textField = new QLineEdit(parent);
                textField->setValidator(new QRegularExpressionValidator(QRegularExpression("-?\\d*"), textField));
                return textField;
            }
            case Parameter::Type::Float: {
                auto textField = new QLineEdit(parent);
                textField->setValidator(new QDoubleValidator(textField));
                return textField;
            }
            // String, BitVector, LogicVector, Time, Enum
            default: {
                return new QLineEdit(parent);
            }
        }
    }

    void ParameterValueDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        if (auto* comboBox = qobject_cast<QComboBox*>(editor))
        {
            model->setData(index, comboBox->currentText(), Qt::EditRole);
        }
        else if (auto* lineEdit = qobject_cast<QLineEdit*>(editor))
        {
            model->setData(index, lineEdit->text(), Qt::EditRole);
        }
    }

    void ParameterValueDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        editor->setGeometry(option.rect);
    }

    Parameter::Type ParameterValueDelegate::parameterType(const QModelIndex& index) const
    {
        const QString typeString = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
        return enum_from_string<Parameter::Type>(typeString.toStdString(), Parameter::Type::String);
    }
}    // namespace hal
