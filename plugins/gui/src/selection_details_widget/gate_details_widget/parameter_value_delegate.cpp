#include "gui/selection_details_widget/gate_details_widget/parameter_value_delegate.h"

#include "gui/selection_details_widget/gate_details_widget/parameter_table_model.h"
#include "hal_core/utilities/enums.h"

#include <QComboBox>
#include <QLineEdit>
#include <QLocale>

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
                textField->setValidator(new QRegularExpressionValidator(
                    QRegularExpression("^-?(?:0[xX][0-9a-fA-F]+|0[oO][0-7]+|0[bB][01]+|\\d+)$"), textField));
                return textField;
            }
            case Parameter::Type::Float: {
                auto textField = new QLineEdit(parent);
                auto validator = new QDoubleValidator(textField);
                QLocale cLocale = QLocale::c();
                cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
                validator->setLocale(cLocale);
                textField->setValidator(validator);
                return textField;
            }
            case Parameter::Type::BitVector: {
                auto textField = new QLineEdit(parent);
                textField->setValidator(new QRegularExpressionValidator(
                    QRegularExpression("^((0[xX][0-9a-fA-F]+)|(0[oO][0-7]+)|(0[bB][01]+))$"), textField));
                return textField;
            }
            case Parameter::Type::LogicVector: {
                auto textField = new QLineEdit(parent);
                textField->setValidator(new QRegularExpressionValidator(
                    QRegularExpression("^((0[xX][0-9a-fA-FxXzZuUlLhHwW-]+)|(0[oO][0-7xXzZuUlLhHwW-]+)|(0[bB][01xXzZuUlLhHwW-]+))$"), textField));
                return textField;
            }
            case Parameter::Type::Time: {
                auto textField = new QLineEdit(parent);
                textField->setValidator(new QRegularExpressionValidator(
                    QRegularExpression("^(?:\\d+\\.?\\d*|\\.\\d+)(fs|ps|ns|us|ms|min|s|h)$"), textField));
                return textField;
            }
            case Parameter::Type::Enum: {
                auto comboBox = new QComboBox(parent);
                comboBox->setFocusPolicy(Qt::StrongFocus);
                comboBox->addItems(index.data(ParameterTableModel::EnumValuesRole).toStringList());
                return comboBox;
            }
            case Parameter::Type::String: {
                return new QLineEdit(parent);
            }
            default: {
                return nullptr;
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
            if (parameterType(index) == Parameter::Type::Integer)
            {
                bool ok               = false;
                const QString decimal = integerToDecimalString(lineEdit->text(), ok);
                if (!ok)
                {
                    return;
                }
                model->setData(index, decimal, Qt::EditRole);
            }
            else
            {
                model->setData(index, lineEdit->text(), Qt::EditRole);
            }
        }
    }

    void ParameterValueDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        editor->setGeometry(option.rect);
    }

    Parameter::Type ParameterValueDelegate::parameterType(const QModelIndex& index)
    {
        const QString typeString = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
        return enum_from_string<Parameter::Type>(typeString.toStdString(), Parameter::Type::String);
    }

    QString ParameterValueDelegate::integerToDecimalString(const QString& text, bool& ok)
    {
        ok = false;

        QString digits = text.trimmed();

        bool negative = false;
        if (digits.startsWith('-'))
        {
            negative = true;
            digits.remove(0, 1);
        }

        int base = 10;
        if (digits.startsWith("0x", Qt::CaseInsensitive))
        {
            base = 16;
            digits.remove(0, 2);
        }
        else if (digits.startsWith("0o", Qt::CaseInsensitive))
        {
            base = 8;
            digits.remove(0, 2);
        }
        else if (digits.startsWith("0b", Qt::CaseInsensitive))
        {
            base = 2;
            digits.remove(0, 2);
        }

        const qlonglong value = digits.toLongLong(&ok, base);
        if (!ok)
        {
            return QString();
        }

        return QString::number(negative ? -value : value);
    }
}    // namespace hal
