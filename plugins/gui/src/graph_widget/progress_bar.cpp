#include "gui/graph_widget/progress_bar.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include <QGridLayout>
#include <QSpacerItem>

namespace hal {

    ProgressBar::ProgressBar(GraphContext *context, QWidget* parent)
        : QFrame(parent)
    {
        setFrameStyle(QFrame::Panel | QFrame::Sunken);
        setLineWidth(3);
        QGridLayout* layout = new QGridLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->addItem(new QSpacerItem(32,32,QSizePolicy::Minimum,QSizePolicy::Expanding),0,0);
        layout->addWidget(mLabel = new QLabel(this), 1, 0);
        layout->addWidget(mProgressBar = new QProgressBar(this), 2, 0);
        layout->addItem(new QSpacerItem(32,32,QSizePolicy::Minimum,QSizePolicy::Expanding),3,0);
        mButAbort = new QPushButton("Abort",this);
        mButAbort->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        layout->addWidget(mButAbort, 4, 0);
        if (context)
            connect(mButAbort,&QPushButton::clicked, context, &GraphContext::abortLayout);
        else
            mButAbort->setDisabled(true);
    }

    void ProgressBar::setValue(int percent)
    {
        mProgressBar->setValue(percent);
    }

    void ProgressBar::setText(const QString& txt)
    {
        mLabel->setText(txt);
    }

}
