#include "gui/selection_details_widget/details_tab_widget.h"

#include "gui/selection_details_widget/details_frame_widget.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel> 
#include <QIcon>
#include <QPixmap>

namespace hal
{
    DetailsTabWidget::DetailsTabWidget(QWidget* parent) : QTabWidget(parent){}

    int DetailsTabWidget::addTab(const QString& label, QList<DetailsFrameWidget*> frames)
    {
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        scrollArea->setWidgetResizable(true);

        QWidget* container = new QWidget(scrollArea);
        container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QVBoxLayout* containerlLayout = new QVBoxLayout(container);
        containerlLayout->setContentsMargins(0, 0, 0, 0);
        containerlLayout->setSpacing(5);
        containerlLayout->setSizeConstraint(QLayout::SetNoConstraint);

        container->setLayout(containerlLayout);
        scrollArea->setWidget(container);

        for(DetailsFrameWidget* frame : frames)
            containerlLayout->addWidget(frame);

        scrollArea->setVerticalScrollBarPolicy(frames.size()>1 ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
        return QTabWidget::addTab(scrollArea, label);
    }

    int DetailsTabWidget::addTab(const QString& label, DetailsFrameWidget* frame)
    {
        QList<DetailsFrameWidget*> frames;
        frames.append(frame);
        return addTab(label,frames);
    }

    void DetailsTabWidget::setIcon(SelectionDetailsIconProvider::IconCategory catg, u32 itemId)
    {
        bool existingContainer = true;
        QLabel* iconContainer = dynamic_cast<QLabel*>(cornerWidget());
        if (!iconContainer)
        {
            existingContainer = false;
            iconContainer = new QLabel(this);
        }

        QPixmap scaledIcon;

        switch (SelectionDetailsIconProvider::sIconSizeSetting->value().toInt())
        {
        case SelectionDetailsIconProvider::NoIcon:
            break;
        case SelectionDetailsIconProvider::SmallIcon:
            scaledIcon = QIcon(*SelectionDetailsIconProvider::instance()->getIcon(catg,itemId)).pixmap(16, 16);
            break;
        case SelectionDetailsIconProvider::BigIcon:
             //assures better quality, icon scaling so smooth, so silky
            scaledIcon = QIcon(*SelectionDetailsIconProvider::instance()->getIcon(catg,itemId)).pixmap(24, 24);
            break;
        }

        iconContainer->setPixmap(scaledIcon);

        if (!existingContainer)
        {
            iconContainer->setContentsMargins(5, 2, 5, 1);
            setCornerWidget(iconContainer);
        }
    }
}
