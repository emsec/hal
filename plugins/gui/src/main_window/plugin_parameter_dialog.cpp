#include "gui/main_window/plugin_parameter_dialog.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "gui/main_window/color_selection.h"
#include "gui/main_window/key_value_table.h"
#include "gui/gui_utils/graphics.h"
#include "gui/gui_globals.h"
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
#include <QTableWidget>

namespace hal {
    PluginParameterDialog::PluginParameterDialog(BasePluginInterface* bpif, QWidget* parent)
        : QDialog(parent), mPluginInterface(bpif)
    {
        setupHash();

        QDialogButtonBox* bbox = setupButtonBox();

        setWindowTitle("Settings for " + QString::fromStdString(mPluginInterface->get_name()) + " plugin");

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
        for (const PluginParameter& par : mParameterList)
        {
            if (par.get_type() != PluginParameter::PushButton) continue;
            QPushButton* but = static_cast<QPushButton*>(mWidgetMap.value(QString::fromStdString(par.get_tagname())));
            if (!but) continue;
            connect(but,&QPushButton::clicked,this,&PluginParameterDialog::handlePushbuttonClicked);
            retval->addButton(but,QDialogButtonBox::ActionRole);
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
            QString parDefault = QString::fromStdString(par.get_value());

            mParameterList.append(par);

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
            case PluginParameter::Dictionary:
            {
                KeyValueTable* kvt = new KeyValueTable(parLabel,this);
                kvt->setJson(par.get_value());
                mWidgetMap[parTagname] = kvt;
                break;
            }
            case PluginParameter::Color:
            {
                mWidgetMap[parTagname] = new ColorSelection(parDefault, parLabel, true);
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
            case PluginParameter::Float:
            {
                QDoubleSpinBox* floatBox = new QDoubleSpinBox(this);
                floatBox->setMaximum(1.E99);
                floatBox->setValue(parDefault.toDouble());
                mWidgetMap[parTagname] = floatBox;
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
        for (const PluginParameter& par : mParameterList)
        {
            QString parTagname = QString::fromStdString(par.get_tagname());
            if (!tabTag.isEmpty() && !parTagname.startsWith(tabTag+ "/")) continue;
            QWidget* widget = mWidgetMap.value(parTagname);
            if (!widget) continue;

            switch (par.get_type())
            {
            // not in form nor tab
            case PluginParameter::PushButton:
                break;
            // without label
            case PluginParameter::Boolean:
            case PluginParameter::Dictionary:
                form->addRow(widget);
                break;
            // label + widget
            default:
                QString parLabel   = QString::fromStdString(par.get_label());
                form->addRow(parLabel, widget);
                break;
            }
        }
    }

    void PluginParameterDialog::accept()
    {
        std::vector<PluginParameter> settings;
        for (PluginParameter par : mParameterList)
        {
            const QWidget* w = mWidgetMap.value(QString::fromStdString(par.get_tagname()));
            if (!w) continue;

            switch (par.get_type())
            {
            case PluginParameter::PushButton:
                qDebug() << "Push" << par.get_tagname().c_str() << par.get_value().c_str();
                break;
            case PluginParameter::Dictionary:
            {
                const KeyValueTable* kvt = static_cast<const KeyValueTable*>(w);
                par.set_value(kvt->toJson());
                break;
            }
            case PluginParameter::Color:
            {
                const ColorSelection* colsel = static_cast<const ColorSelection*>(w);
                par.set_value(colsel->colorName().toStdString());
                break;
            }
            case PluginParameter::Boolean:
            {
                const QCheckBox* check = static_cast<const QCheckBox*>(w);
                par.set_value(check->checkState()==Qt::Checked ? "true" : "false");
                break;
            }
            case PluginParameter::Integer:
            {
                const QSpinBox* intBox = static_cast<const QSpinBox*>(w);
                par.set_value(QString::number(intBox->value()).toStdString());
                break;
            }
            case PluginParameter::Float:
            {
                const QDoubleSpinBox* floatBox = static_cast<const QDoubleSpinBox*>(w);
                par.set_value(QString::number(floatBox->value()).toStdString());
                break;
            }
            case PluginParameter::String:
            {
                const QLineEdit* ledit = static_cast<const QLineEdit*>(w);
                par.set_value(ledit->text().toStdString());
                break;
            }
            case PluginParameter::ExistingDir:
            case PluginParameter::NewFile:
            {
                const PluginParameterFileDialog* fileDlg = static_cast<const PluginParameterFileDialog*>(w);
                par.set_value(fileDlg->getFilename().toStdString());
                break;
            }
            default:
                continue;
                break;
            }
            settings.push_back(par);
        }
        QDialog::accept();
        mPluginInterface->set_parameter(gNetlist, settings);
    }

    void PluginParameterDialog::handlePushbuttonClicked()
    {
        for (PluginParameter& par : mParameterList)
            if (par.get_type() == PluginParameter::PushButton)
            {
                const QWidget* w = mWidgetMap.value(QString::fromStdString(par.get_tagname()));
                if (sender() == w)
                {
                    par.set_value("clicked");
                }
            }
        accept();
    }

    PluginParameterFileDialog::PluginParameterFileDialog(const PluginParameter& par, QWidget* parent)
        : QWidget(parent), mParameter(par)
    {
        QGridLayout* layout = new QGridLayout(this);
        QString parDefault = QString::fromStdString(mParameter.get_value());
        mEditor = new QLineEdit(this);
        mEditor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mEditor->setMinimumWidth(320);
        mEditor->setText(parDefault);
        layout->addWidget(mEditor,0,0);

        QString iconPath = (mParameter.get_type() == PluginParameter::ExistingDir)
                ? ":/icons/folder"
                : ":/icons/folder-down";
        mButton = new QPushButton(gui_utility::getStyledSvgIcon("all->#3192C5",iconPath),"",this);
        mButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
        connect(mButton,&QPushButton::clicked,this,&PluginParameterFileDialog::handleActivateFileDialog);
        layout->addWidget(mButton,0,1);
    }

    void PluginParameterFileDialog::handleActivateFileDialog()
    {
        QString parLabel = QString::fromStdString(mParameter.get_label());
        QString parDefault = QString::fromStdString(mParameter.get_value());
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
