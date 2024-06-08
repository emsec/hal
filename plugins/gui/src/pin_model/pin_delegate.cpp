#include "gui/pin_model/pin_delegate.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

#include <QComboBox>
#include <QApplication>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
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

        // Column 0: Name, Column 1: Type, Column 2: Direction, Column 3: Delete button
        switch(index.column()){
            case 0:{
                auto lineEdit = new QLineEdit(parent);
                lineEdit->setFocusPolicy(Qt::StrongFocus);
                return lineEdit;
            }
            case 1: {
                //dont create a widget for dummyEntries
                if(itemType == PinItem::TreeItemType::GroupCreator || itemType == PinItem::TreeItemType::PinCreator)
                    return nullptr;
                auto comboBox = new QComboBox(parent);
                //TODO provide enum to string method
                comboBox->addItem("input");
                comboBox->addItem("output");
                comboBox->addItem("inout");
                comboBox->addItem("internal");
                comboBox->setFocusPolicy(Qt::StrongFocus);
                return comboBox;
            }
            case 2:{
                //dont create a widget for dummyEntries
                if(itemType == PinItem::TreeItemType::GroupCreator || itemType == PinItem::TreeItemType::PinCreator)
                    return nullptr;

                auto comboBox = new QComboBox(parent);
                //TODO provide enum to string method
                comboBox->addItem("none");
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
                comboBox->setFocusPolicy(Qt::StrongFocus);
                return comboBox;
            }
            case 3:{
                qInfo() << "create editor for 3";
            }
        }


        return nullptr;
    }

    void PinDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        //TODO set editor data if direction or type is selected
        //qInfo() << "Editing row: " << index.row() << "  column: " << index.column();
        auto pinItem = static_cast<PinItem*>(index.internalPointer());
        PinItem::TreeItemType itemType = pinItem->getItemType();

        switch(index.column()){
            case 0: {
                //name editor
                auto lineEdit = static_cast<QLineEdit*>(editor);
                if(itemType != PinItem::TreeItemType::PinCreator && itemType != PinItem::TreeItemType::GroupCreator){
                    lineEdit->setText(pinItem->getName());
                }
                break;
            }
            case 1: {
                //direction editor
                //TODO currently only pins can change the direction while groups does not allow modification
                if(itemType != PinItem::TreeItemType::Pin && itemType != PinItem::TreeItemType::InvalidPin)
                    break;
                auto comboBox = static_cast<QComboBox*>(editor);
                comboBox->setCurrentText(pinItem->getDirectionAsText());
                break;
            }
            case 2: {
                //type editor
                if(itemType == PinItem::TreeItemType::PinCreator || itemType == PinItem::TreeItemType::GroupCreator)
                    break;
                auto comboBox = static_cast<QComboBox*>(editor);
                comboBox->setCurrentText(pinItem->getPinTypeAsText());
                break;
            }
        }

    }

    void PinDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        //TODO cast editor to corresponding column
        PinItem::TreeItemType itemType = static_cast<PinItem*>(index.internalPointer())->getItemType();
        auto pinModel = static_cast<PinModel*>(model);

        switch(index.column()){
            case 0: {
                //name column
                auto lineEdit = static_cast<QLineEdit*>(editor);
                QString text  = lineEdit->text();
                if (!text.isEmpty())
                    pinModel->handleEditName(index, text);
                break;
            }
            case 1:{
                //direction column
                //TODO currently only pins can change the direction while groups does not allow modification
                if(itemType == PinItem::TreeItemType::GroupCreator || itemType == PinItem::TreeItemType::PinCreator)
                    break;
                auto comboBox = static_cast<QComboBox*>(editor);
                QString text = comboBox->currentText();
                if(!text.isEmpty())
                    pinModel->handleEditDirection(index, text);
                break;
            }
            case 2:{
                //type column
                if(itemType == PinItem::TreeItemType::GroupCreator || itemType == PinItem::TreeItemType::PinCreator)
                    break;
                auto comboBox = static_cast<QComboBox*>(editor);
                QString text = comboBox->currentText();
                if(!text.isEmpty())
                    pinModel->handleEditType(index, text);
                break;
            }
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
        if(itemType == PinItem::TreeItemType::InvalidPin || itemType == PinItem::TreeItemType::InvalidPinGroup){
            painter->fillRect(option.rect, Qt::darkRed);
        }
        QStyledItemDelegate::paint(painter, option, index);
    }
}
