#include "gui/main_window/plugin_parameter_dialog.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "gui/main_window/color_selection.h"
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QFileDialog>

namespace hal {
    PluginParameterDialog::PluginParameterDialog(BasePluginInterface* bpif, QWidget* parent)
        : QDialog(parent), mPluginInterface(bpif)
    {
        setupHash();

        QDialogButtonBox* bbox = setupButtonBox();

        if (mTabNames.isEmpty())
        {
            QFormLayout* layout = new QFormLayout(this);
            setupForm(layout);
            layout->addRow(bbox);
        }
        else
        {
            QGridLayout* grid = new QGridLayout(this);
            QTabWidget* tabw = new QTabWidget(this);
            for (auto it = mTabNames.constBegin(); it != mTabNames.constEnd(); ++it)
            {
                QWidget* tab = new QWidget(tabw);
                QFormLayout* layout = new QFormLayout(tab);
                setupForm(layout,it.key());
                tabw->addTab(tab,it.value());
            }
            grid->addWidget(tabw,0,0,1,2);
            grid->addWidget(bbox,1,1);
        }
    }

    QDialogButtonBox* PluginParameterDialog::setupButtonBox()
    {
        QDialogButtonBox* retval = new QDialogButtonBox(QDialogButtonBox::Cancel,this);
        for (auto it = mParameterMap.constBegin(); it != mParameterMap.constEnd(); ++it)
        {
            if (it.value().get_type() != PluginParameter::PushButton) continue;
            QPushButton* but = static_cast<QPushButton*>(mWidgetMap.value(it.key()));
            if (!but) continue;
            retval->addButton(but,QDialogButtonBox::AcceptRole);
        }
        connect(retval,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(retval,&QDialogButtonBox::accepted,this,&PluginParameterDialog::accept);
        return retval;
    }

    void PluginParameterDialog::setupHash()
    {
        for (PluginParameter par : mPluginInterface->get_parameter())
        {
            if (par.get_type() == PluginParameter::Absent) continue;
            QString parTagname = QString::fromStdString(par.get_tagname());
            QString parLabel   = QString::fromStdString(par.get_label());
            QString parDefault = QString::fromStdString(par.get_default_value());

            mParameterMap[parTagname] = par;

            QStringList tabbed = parTagname.split('/');
            if (tabbed.size() > 1)
            {
                QString tabTag = tabbed.at(0);
                if (!mTabNames.contains(tabTag))
                    mTabNames[tabTag] = tabTag;
            }

            switch (par.get_type())
            {
            case PluginParameter::TabName:
                mTabNames[parTagname] = parLabel;
                break;
            case PluginParameter::PushButton:
                mWidgetMap[parTagname] = new QPushButton(parLabel,this);
                break;
            case PluginParameter::Color:
            {
                mWidgetMap[parTagname] = new ColorSelection("#A0FFE0", parLabel, true);
                break;
            }
            case PluginParameter::Boolean:
            {
                QCheckBox* check = new QCheckBox(parLabel,this);
                check->setChecked(parDefault.toLower()=="true");
                mWidgetMap[parTagname] = check;
                break;
            }
            case PluginParameter::Integer:
            {
                QSpinBox* intBox = new QSpinBox(this);
                intBox->setMaximum(1000000000);
                intBox->setValue(parDefault.toInt());
                mWidgetMap[parTagname] = intBox;
                break;
            }
            case PluginParameter::String:
            {
                QLineEdit* ledit = new QLineEdit(this);
                ledit->setText(parDefault);
                mWidgetMap[parTagname] = ledit;
                break;
            }
            case PluginParameter::ExistingDir:
            case PluginParameter::NewFile:
                mWidgetMap[parTagname] = new PluginParameterFileDialog(par,this);
                break;
            default:
                break;
            }
        }
    }

    void PluginParameterDialog::setupForm(QFormLayout* form, const QString& tabTag)
    {
        for (auto it = mParameterMap.constBegin(); it != mParameterMap.constEnd(); ++it)
        {
            QString parTagname = it.key();
            if (!tabTag.isEmpty() && !parTagname.startsWith(tabTag+ "/")) continue;
            QWidget* widget = mWidgetMap.value(it.key());
            if (!widget) continue;

            switch (it.value().get_type())
            {
            case PluginParameter::PushButton:
                break;
            case PluginParameter::Boolean:
                form->addRow(widget);
                break;
            default:
                QString parLabel   = QString::fromStdString(it.value().get_label());
                form->addRow(parLabel, widget);
                break;
            }

            QString parLabel   = QString::fromStdString(it.value().get_label());
        }
    }

    void PluginParameterDialog::accept()
    {
        qDebug() << "PluginParameterDialog::accept()";
        QDialog::accept();
    }

    void PluginParameterDialog::handlePushbuttonClicked()
    {
        qDebug() << "PluginParameterDialog::handlePushbuttonClicked()";
    }

    PluginParameterFileDialog::PluginParameterFileDialog(const PluginParameter& par, QWidget* parent)
        : QWidget(parent), mParameter(par)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        QString parDefault = QString::fromStdString(mParameter.get_default_value());
        mEditor = new QLineEdit(this);
        mEditor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mEditor->setText(parDefault);
        layout->addWidget(mEditor);

        mButton = new QPushButton("v",this);
        mButton->setFixedWidth(32);
        connect(mButton,&QPushButton::clicked,this,&PluginParameterFileDialog::handleActivateFileDialog);
        layout->addWidget(mButton);
    }

    void PluginParameterFileDialog::handleActivateFileDialog()
    {
        QString parLabel = QString::fromStdString(mParameter.get_label());
        QString parDefault = QString::fromStdString(mParameter.get_default_value());
        QString dir = QFileInfo(parDefault).isDir() ? parDefault : QFileInfo(parDefault).path();
        QString filename = (mParameter.get_type() == PluginParameter::ExistingDir)
                ? QFileDialog::getExistingDirectory(this,parLabel,dir)
                : QFileDialog::getSaveFileName(this,parLabel,dir);
        if (!filename.isEmpty())
            mEditor->setText(filename);
    }

    QString PluginParameterFileDialog::getFilename() const
    {
        return mEditor->text();
    }
}
