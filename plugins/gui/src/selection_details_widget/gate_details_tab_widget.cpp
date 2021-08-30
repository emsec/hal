#include "gui/selection_details_widget/gate_details_tab_widget.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel> //debug


namespace hal
{
    GateDetailsTabWidget::GateDetailsTabWidget(QWidget* parent) : QTabWidget(parent)
    {
        
        //workflow for single tab

        //low level widgets (tables, trees, etc...) but as members in production
        QLabel* t1 = new QLabel("Ich bin eine Tabelle1 mit ziemlich viel inhalt balbalblalbalblab", this);
        QLabel* t2 = new QLabel("Ich bin eine Tabelle2 mit ziemlich viel inhalt balbalblalbalblab", this); 
        QLabel* t3 = new QLabel("Ich bin eine Tabelle3 mit ziemlich viel inhalt balbalblalbalblab", this); 
        QLabel* t4 = new QLabel("Ich bin eine Tabelle4 mit ziemlich viel inhalt balbalblalbalblab", this); 

        //frames for low level widgets
        DetailsFrameWidget* frame1 = new DetailsFrameWidget(t1, "test", this);
        DetailsFrameWidget* frame2 = new DetailsFrameWidget(t2, "test", this);
        DetailsFrameWidget* frame3 = new DetailsFrameWidget(t3, "test", this);
        DetailsFrameWidget* frame4 = new DetailsFrameWidget(t4, "test", this);

        //container for tab content
        QWidget* container = new QWidget(this);
        container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        //layout for tab content container
        QVBoxLayout* containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->setSpacing(30);
        containerLayout->setSizeConstraint(QLayout::SetNoConstraint);

        container->setLayout(containerLayout); //do we need this line or is line 34 sufficient?
        containerLayout->addWidget(frame1);
        containerLayout->addWidget(frame2);
        containerLayout->addWidget(frame3);
        containerLayout->addWidget(frame4);

        //scroll area for container
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        scrollArea->setWidgetResizable( true );
        scrollArea->setWidget(container);

        //put container in tab
        addTab(scrollArea, "Test1");

        //connect frames signal slots
    }

    void GateDetailsTabWidget::setGate(hal::Gate* gate)
    {
        //member widgets of lowest level have setGate as interface, pass it to theme here
    }
}
