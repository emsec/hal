#include "gui/pin_model/pin_delegate.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
#include <gui/pin_model/pin_item.h>
#include <gui/pin_model/pin_model.h>

namespace hal
{

    PinDelegate::PinDelegate(QObject* parent) : QStyledItemDelegate(parent)
    {

    }

    QWidget* PinDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        //TODO create editors
        switch(index.column()){
            case 0:{
                QLineEdit* lineEdit = new QLineEdit(parent);
                return lineEdit;
            }
            case 1: {
                QComboBox* comboBox = new QComboBox(parent);
                comboBox->addItem("input");
                comboBox->addItem("output");
                comboBox->addItem("inout");
                comboBox->addItem("internal");
                return comboBox;
            }
            case 2:{
                QComboBox* comboBox = new QComboBox(parent);
                comboBox->addItem("power");
                comboBox->addItem("ground");
                comboBox->addItem("lut");
                return comboBox;
            }
        }



    }

    void PinDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        qInfo() << "Editing row: " << index.row() << "  column: " << index.column() << "   " << index.data().toString();
    }

    void PinDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        //TODO cast editor to corresponding column

        if(index.column() == 0)
        {
            auto lineEdit = static_cast<QLineEdit*>(editor);
            QString text  = lineEdit->text();
            if (!text.isEmpty())
                static_cast<PinModel*>(model)->handleEdit(index, text);
        }
    }


    void PinDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        //QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        editor->setGeometry(option.rect);
    }
}
