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

        return QTabWidget::addTab(scrollArea, label);
    }

    void DetailsTabWidget::setIcon(const QString& fileName)
    {
        QIcon* icon = new QIcon(fileName);
        QPixmap scaledIcon = icon->pixmap(24, 24); //assures better quality, icon scaling so smooth, so silky

        QLabel* iconContainer = new QLabel(this);
        iconContainer->setPixmap(scaledIcon);
        iconContainer->setContentsMargins(5, 2, 5, 1);

        setCornerWidget(iconContainer);
    }
}
