#include "waveform_viewer/simulation_setting_dialog.h"
#include <QGridLayout>
#include <QFormLayout>
#include <QDebug>
#include <QHeaderView>
#include <QPainter>
#include <QLabel>

namespace hal {

    SimulationSettingDialog::SimulationSettingDialog(SimulationSettings *settings, QWidget* parent)
        : QDialog(parent), mSettings(settings)
    {
        QGridLayout* layout = new QGridLayout(this);
        mTabWidget = new QTabWidget(this);
        mTabWidget->addTab(new SimulationSettingGlobalTab(settings,this), "Global settings");
        mTabWidget->addTab(new SimulationSettingPropertiesTab(settings,this), "Engine properties");
        mTabWidget->addTab(new SimulationSettingColorTab(settings,this), "Color settings");
        mTabWidget->setMinimumWidth(640);
        layout->addWidget(mTabWidget,0,0,1,2);
        QDialogButtonBox* mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &SimulationSettingDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &SimulationSettingDialog::reject);
        layout->addWidget(mButtonBox,1,1);
    }

    void SimulationSettingDialog::accept()
    {
        const SimulationSettingGlobalTab* globTab = static_cast<const SimulationSettingGlobalTab*>(mTabWidget->widget(0));
        mSettings->setMaxSizeLoadable(globTab->maxSizeLoadable());
        mSettings->setMaxSizeEditor(globTab->maxSizeEditor());
        mSettings->setBaseDirectory(globTab->isCustomBaseDirectory() ? globTab->baseDirectory() : QString(""));

        const SimulationSettingPropertiesTab* propTab = static_cast<const SimulationSettingPropertiesTab*>(mTabWidget->widget(1));
        mSettings->setEngineProperties(propTab->engineProperties());

        const SimulationSettingColorTab* colorTab = static_cast<const SimulationSettingColorTab*>(mTabWidget->widget(2));
        for (int irow = 0; irow < SimulationSettings::MaxColorSetting; irow++)
        {
             SimulationSettings::ColorSetting cs = (SimulationSettings::ColorSetting)irow;
             mSettings->setColor(cs,colorTab->colorSetting(cs));
        }
        mSettings->sync();

        QDialog::accept();
    }

    //-----------------------------------
    SimulationSettingColorTab::SimulationSettingColorTab(SimulationSettings *settings, QWidget* parent)
        : QWidget(parent)
    {
        QFormLayout* layout = new QFormLayout(this);
        const char* labl[SimulationSettings::MaxColorSetting] = {"regular waveform", "selected waveform", "waveform value undefined", "value X", "value 0", "value 1"};
        for (int irow = 0; irow < SimulationSettings::MaxColorSetting; irow++)
        {
            switch (irow)
            {
            case 0:
                layout->addRow(new QLabel("Color for waveform graph:", this));
                break;
            case 3:
                QLabel* lab = new QLabel("Background color for values", this);
                lab->setMinimumHeight(50);
                lab->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
                layout->addRow(lab);
                break;
            }

            mActivateColorDialog[irow] = new SimulationSettingColorButton(settings->color((SimulationSettings::ColorSetting)irow), irow>=3, this);
            mActivateColorDialog[irow]->setMaximumSize(QSize(32,24));
            mActivateColorDialog[irow]->setAutoFillBackground(true);
            connect(mActivateColorDialog[irow],&QPushButton::clicked,this,&SimulationSettingColorTab::activateColorDialog);
            layout->addRow(QString(labl[irow]),mActivateColorDialog[irow]);
        }
    }

    SimulationSettingColorButton::SimulationSettingColorButton(const QString& col, bool bullet, QWidget* parent)
        : QPushButton(" ", parent), mColorName(col), mBullet(bullet) {;}

    void SimulationSettingColorButton::paintEvent(QPaintEvent* evt)
    {
        Q_UNUSED(evt);
        QPainter painter(this);

        painter.setPen(QPen(Qt::black,0));
        QRectF r = rect();
        if (mBullet)
        {
            painter.setBrush(QBrush(QColor(mColorName)));
            int delta = (r.width() - r.height()) / 2;
            if (delta > 0)
            {
                r.setLeft(r.left() + delta);
                r.setRight(r.right() - delta);
            }
            else if (delta < 0)
            {
                r.setTop(r.top() - delta);
                r.setBottom(r.bottom() + delta);
            }
            painter.drawEllipse(r);
        }
        else
        {
            painter.drawRect(r);
            painter.setPen(QPen(QColor(mColorName),5.));
            int yc = r.top() + r.height()/2;
            painter.drawLine(r.left(),yc,r.right(),yc);
        }
    }

    void SimulationSettingColorTab::activateColorDialog()
    {
        QObject* obj = sender();
        for (int irow = 0; irow < SimulationSettings::MaxColorSetting; irow++)
        {
            if (mActivateColorDialog[irow] == obj)
            {
                QColor currentColor = colorSetting(irow);
                QColor selectedColor = QColorDialog::getColor(currentColor, this, "Select color for " + mActivateColorDialog[irow]->text());
                if (selectedColor.isValid() && selectedColor != currentColor)
                {
                    mActivateColorDialog[irow]->mColorName = selectedColor.name();
                    mActivateColorDialog[irow]->update();
                }
            }
        }
    }

    QString SimulationSettingColorTab::colorSetting(int inx) const
    {
        return mActivateColorDialog[inx]->mColorName;
    }

    //-----------------------------------
    SimulationSettingGlobalTab::SimulationSettingGlobalTab(SimulationSettings* settings, QWidget* parent)
        : QWidget(parent)
    {
        QFormLayout* layout = new QFormLayout(this);
        mMaxSizeLoadable = new QSpinBox(this);
        mMaxSizeLoadable->setMaximum(5000000);
        mMaxSizeLoadable->setMinimum(50);
        mMaxSizeLoadable->setValue(settings->maxSizeLoadable());
        layout->addRow("Load waveform to memory if number transitions <", mMaxSizeLoadable);
        mMaxSizeEditor = new QSpinBox(this);
        mMaxSizeEditor->setMaximum(5000);
        mMaxSizeEditor->setMinimum(10);
        mMaxSizeEditor->setValue(settings->maxSizeLoadable());
        layout->addRow("Maximum number of values to load into editor ", mMaxSizeEditor);
        layout->addItem(new QSpacerItem(0,100));
        mCustomBaseDicectory = new QCheckBox(this);
        connect(mCustomBaseDicectory,&QCheckBox::toggled,this,&SimulationSettingGlobalTab::customBaseDirectoryToggled);
        layout->addRow("Custom base directory instead of project dir:", mCustomBaseDicectory);
        mEditBaseDirectory = new QLineEdit(this);
        mEditBaseDirectory->setText(settings->baseDirectory());
        layout->addRow("Base directory:", mEditBaseDirectory);
    }

    void SimulationSettingGlobalTab::customBaseDirectoryToggled(bool on)
    {
        mEditBaseDirectory->setEnabled(on);
    }

    //-----------------------------------
    SimulationSettingPropertiesTab::SimulationSettingPropertiesTab(SimulationSettings* settings, QWidget* parent)
        : QTableWidget(parent)
    {
        QMap<QString,QString> engProp = settings->engineProperties();
        setColumnCount(2);
        setColumnWidth(0,250);
        setColumnWidth(1,350);
        setRowCount(engProp.size()+3);
        setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
        int irow = 0;
        for (auto it = engProp.constBegin(); it != engProp.constEnd(); ++it)
        {
            QTableWidgetItem wi(it.key());
            setItem(irow,0,new QTableWidgetItem(it.key()));
            setItem(irow,1,new QTableWidgetItem(it.value()));
            ++irow;
        }
        horizontalHeader()->setStretchLastSection(true);
        connect(this,&QTableWidget::cellChanged,this,&SimulationSettingPropertiesTab::handleCellChanged);
    }

    void SimulationSettingPropertiesTab::handleCellChanged(int irow, int icolumn)
    {
        if ((icolumn == 1 && irow >= rowCount()-2) ||
            (icolumn == 0 && irow >= rowCount()-1))
            setRowCount(rowCount()+1);
    }

    QMap<QString,QString> SimulationSettingPropertiesTab::engineProperties() const
    {
        QMap<QString,QString> retval;
        for (int irow=0; irow < rowCount(); ++irow)
        {
            const QTableWidgetItem* wi = item(irow,0);
            if (!wi) continue;
            QString key = wi->text().trimmed();
            if (key.isEmpty()) continue;
            wi = item(irow,1);
            QString value = wi ? wi->text().trimmed() : QString();
            retval[key] = value;
        }
        return retval;
    }
}
