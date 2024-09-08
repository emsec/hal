
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_pin.h"

#include "gui/gui_globals.h"

#include <QTreeView>
#include <gui/pin_model/pin_model.h>
#include <gui/pin_model/pin_delegate.h>

namespace hal
{

    PinTreeView::PinTreeView(QWidget* parent)
        : QTreeView(parent)
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
        setSelectionBehavior(QAbstractItemView::SelectItems);
    }

    /*
    QModelIndex PinTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    {
        QModelIndex inx = currentIndex();
        QModelIndex par = inx.parent();
        int pcount = 0;
        while (par.isValid())
        {
            ++pcount;
            par = par.parent();
        }
        std::cerr << "moveCursor " << cursorAction << " current: " << model()->data(inx).toString().toStdString() << std::endl;

        return QTreeView::moveCursor(cursorAction, modifiers);
    }
    */

    void PinTreeView::handleEditNewDone(const QModelIndex& index)
    {
        QModelIndex nextIndex;

        if (index.column() || index.parent().isValid())
            nextIndex = model()->index(index.row()+1,0,index.parent());
        else
            nextIndex = model()->index(index.row(),1,index.parent());

        setCurrentIndex(nextIndex);
        edit(nextIndex);
    }


    GateLibraryTabPin::GateLibraryTabPin(bool editable, QWidget* parent)
        : GateLibraryTabInterface(parent)
    {
        mGridLayout = new QGridLayout(this);

        mTreeView   = new PinTreeView(this);

        if (editable)
        {
            mPinModel = new PinModel(this, editable);
            auto pinDelegate = new PinDelegate(this);
            mTreeView->setItemDelegate(pinDelegate);
            connect(mPinModel, &PinModel::editNewDone, mTreeView, &PinTreeView::handleEditNewDone, Qt::QueuedConnection);
        }
        else
        {
            mPinModel = new PinModel(this);
        }

        mTreeView->setModel(mPinModel);

        mGridLayout->addWidget(mTreeView);

        setLayout(mGridLayout);
    }

    void GateLibraryTabPin::update(GateType* gate)
    {

        mPinModel->setGate(gate);

        //TODO resize to see children columns
        mTreeView->resizeColumnToContents(0);
    }

    QTreeView* GateLibraryTabPin::getTreeView()
    {
        return mTreeView;
    }

    PinModel* GateLibraryTabPin::getPinModel()
    {
        return mPinModel;
    }

}
