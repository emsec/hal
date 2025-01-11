#include "gui/graph_widget/progress_bar.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QImage>
#include <QApplication>
#include <QPainter>
#include <math.h>
#include <QDebug>

namespace hal {

    ProgressBar::ProgressBar(GraphContext *context, QWidget* parent)
        : AbstractBusyIndicator(parent)
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
        if (percent >= 6) mButAbort->setDisabled(true);
        mProgressBar->setValue(percent);
    }

    void ProgressBar::setText(const QString& txt)
    {
        mLabel->setText(txt);
    }
//----------------------

    BusyIndicator::BusyIndicator(QWidget* parent)
        : AbstractBusyIndicator(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignHCenter);
        layout->addWidget(mLabel = new QLabel(this));
        mAnimation = new BusyAnimation(this);
        mTimer = new QTimer(this);
        connect(mTimer,&QTimer::timeout,mAnimation,&BusyAnimation::handleTimeout);
        layout->addWidget(mAnimation);
        layout->addWidget(mProgressBar = new QProgressBar(this));
        mTimer->start(80);
    }

    BusyIndicator::~BusyIndicator()
    {
        mTimer->stop();
    }

    BusyAnimation::BusyAnimation(QWidget* parent)
        : QWidget(parent), mImage(QImage(":/icons/hal9000","PNG")), mAngle(0)
    {;}

    void BusyAnimation::handleTimeout()
    {
        mAngle += 5;
        update();
        qApp->processEvents();
    }

    void BusyAnimation::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int rw = rect().width();
        int rh = rect().height();

        int w = rw > rh ? rh : rw;
        int h = w;

        QRect rimg((rw-w)/2,(rh-h)/2,w,h);

//        p.fillRect(rimg,QBrush(Qt::gray));
        QImage img = mImage.scaled(w,h);
        double xc = w / 2.;
        double yc = h / 2.;

        for (int y = 0; y<h; y++)
            for (int x=0; x<w; x++)
            {
                QRgb col = img.pixel(x,y);
                if (col & 0xFF000000)
                {
                    double angle = atan2(y-yc,x-xc) / M_PI * 180.;
                    double da = angle >= mAngle ? angle - mAngle : angle + 360. - mAngle;
                    int opaque = floor(da / 360.*256);
                    opaque <<= 24;
                    col = (col & 0xFFFFFF) | opaque;
                    img.setPixel(x,y,col);
                }
            }

        p.drawImage(rimg,img);
    }

    void BusyIndicator::setValue(int percent)
    {
        mProgressBar->setValue(percent);
        update();
        qApp->processEvents();
    }

    void BusyIndicator::setText(const QString &txt)
    {
        if (mLabel->text() == txt) return;
        mLabel->setText(txt);
        qApp->processEvents();
    }
}
