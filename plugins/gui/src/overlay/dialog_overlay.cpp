#include "gui/overlay/dialog_overlay.h"

#include "gui/dialogs/dialog.h"

#include <QVBoxLayout>

namespace hal
{
    DialogOverlay::DialogOverlay(QWidget* parent) : Overlay(parent),
        //mLayout(new QVBoxLayout(this)),
        mDialog(nullptr)
    {
    }

    void DialogOverlay::setDialog(Dialog* dialog)
    {
        if (mDialog)
        {
            mDialog->hide();
            mDialog->setParent(nullptr);
        }

        mDialog = dialog;

        dialog->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //mLayout->addWidget(dialog, Qt::AlignCenter);
        //mLayout->setAlignment(dialog, Qt::AlignCenter);

        dialog->setParent(this);

        mDialog->show();

        // OLD
        //    mWidget->setParent(this);
        //    mWidget->resize(mWidget->sizeHint());

        connect(mDialog, &Dialog::offsetChanged, this, &DialogOverlay::positionDialog);
    }

    void DialogOverlay::resizeEvent(QResizeEvent* event)
    {
        Q_UNUSED(event)

        positionDialog();
    }

    void DialogOverlay::positionDialog()
    {
        int x = width() / 2 - mDialog->width() / 2 + mDialog->xOffset();
        int y = height() / 2 - mDialog->height() / 2 + mDialog->yOffset();

        mDialog->move(x, y);
    }
}
