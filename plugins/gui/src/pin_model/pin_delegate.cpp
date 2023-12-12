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
        PinItem::TreeItemType itemType = static_cast<PinItem*>(index.internalPointer())->getItemType();
        //TODO create editors
        switch(index.column()){
            case 0:{
                QLineEdit* lineEdit = new QLineEdit(parent);
                return lineEdit;
            }
            case 1: {
                if(itemType != PinItem::TreeItemType::Pin)
                    return new QWidget(parent);
                QComboBox* comboBox = new QComboBox(parent);
                //TODO provide enum to string method
                comboBox->addItem("input");
                comboBox->addItem("output");
                comboBox->addItem("inout");
                comboBox->addItem("internal");
                return comboBox;
            }
            case 2:{
                if(itemType != PinItem::TreeItemType::Pin)
                    return new QWidget(parent);
                QComboBox* comboBox = new QComboBox(parent);
                //TODO provide enum to string method
                comboBox->addItem("power");
                comboBox->addItem("ground");
                comboBox->addItem("lut");
                comboBox->addItem("state");
                comboBox->addItem("neg_state");
                comboBox->addItem("clock");
                comboBox->addItem("enable");
                comboBox->addItem("set");
                comboBox->addItem("reset");
                comboBox->addItem("data");
                comboBox->addItem("address");
                comboBox->addItem("io_pad");
                comboBox->addItem("select");
                comboBox->addItem("carry");
                comboBox->addItem("sum");
                return comboBox;
            }
        }



    }

    void PinDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        //TODO set editor data if direction or type is selected
        qInfo() << "Editing row: " << index.row() << "  column: " << index.column();

    }

    void PinDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        //TODO cast editor to corresponding column

        if(index.column() == 0)
        {
            auto lineEdit = static_cast<QLineEdit*>(editor);
            QString text  = lineEdit->text();
            if (!text.isEmpty())
                static_cast<PinModel*>(model)->handleEditName(index, text);
        }
    }


    void PinDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        //QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        editor->setGeometry(option.rect);
    }

    void PinDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        //TODO colorize dummy entries and invalid pins

        PinItem::TreeItemType itemType = static_cast<PinItem*>(index.internalPointer())->getItemType();
        if(itemType == PinItem::TreeItemType::InvalidPin){
            painter->fillRect(option.rect, Qt::darkRed);
        }
        QStyledItemDelegate::paint(painter, option, index);
    }
}
