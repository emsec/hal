#include "gui/selection_details_widget/details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"

#include <QVBoxLayout>
#include <QScrollArea> 

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
}
