#include "gui/main_window/plugin_parameter_dialog.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/module_dialog/gate_dialog.h"
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
#include <QComboBox>

namespace hal {
    PluginParameterDialog::PluginParameterDialog(const QString &pname, GuiExtensionInterface *geif, QWidget* parent)
        : QDialog(parent), mPluginName(pname), mGuiExtensionInterface(geif)
    {
        setupHash();

        QDialogButtonBox* bbox = setupButtonBox();

        setWindowTitle("Settings for " + mPluginName + " plugin");

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
            if (par.get_type() != PluginParameter::PushButton || par.get_tagname().find('/') != std::string::npos) continue;
            QPushButton* but = static_cast<QPushButton*>(mWidgetMap.value(QString::fromStdString(par.get_tagname())));
            Q_ASSERT(but);
            connect(but,&QPushButton::clicked,this,&PluginParameterDialog::handlePushbuttonClicked);
            retval->addButton(but,QDialogButtonBox::ActionRole);
        }
        connect(retval,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(retval,&QDialogButtonBox::accepted,this,&PluginParameterDialog::accept);
        return retval;
    }

    void PluginParameterDialog::setupHash()
    {
        if (!mGuiExtensionInterface) return;
        for (PluginParameter par : mGuiExtensionInterface->get_parameter())
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
            case PluginParameter::ExistingFile:
            case PluginParameter::NewFile:
                mWidgetMap[parTagname] = new PluginParameterFileDialog(par,this);
                break;
            case PluginParameter::Module:
                mWidgetMap[parTagname] = new PluginParameterNodeDialog(par,this);
                break;
            case PluginParameter::Gate:
                mWidgetMap[parTagname] = new PluginParameterNodeDialog(par,this);
                break;
            case PluginParameter::ComboBox:
            {
                QComboBox* cbox = new QComboBox(this);
                cbox->insertItems(0,QString::fromStdString(par.get_value()).split(';'));
                if (!par.get_value().empty()) cbox->setCurrentIndex(0);
                mWidgetMap[parTagname] = cbox;
                break;
            }
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
            // push button not in tab are located in button box
            case PluginParameter::PushButton:
                if (par.get_tagname().find('/')!=std::string::npos)
                {
                    QPushButton* but = static_cast<QPushButton*>(widget);
                    Q_ASSERT(but);
                    connect(but,&QPushButton::clicked,this,&PluginParameterDialog::handlePushbuttonClicked);
                    form->addRow(but);
                }
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
        if (!mGuiExtensionInterface) return;
        std::vector<PluginParameter> settings;
        std::string buttonClicked;
        for (PluginParameter par : mParameterList)
        {
            const QWidget* w = mWidgetMap.value(QString::fromStdString(par.get_tagname()));
            if (!w) continue;

            switch (par.get_type())
            {
            case PluginParameter::PushButton:
                if (par.get_value() == "clicked")
                {
                    buttonClicked = par.get_tagname();
                }
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
            case PluginParameter::ExistingFile:
            case PluginParameter::NewFile:
            {
                const PluginParameterFileDialog* fileDlg = static_cast<const PluginParameterFileDialog*>(w);
                par.set_value(fileDlg->getFilename().toStdString());
                break;
            }
            case PluginParameter::Gate:
            case PluginParameter::Module:
            {
                const PluginParameterNodeDialog* nodeDlg = static_cast<const PluginParameterNodeDialog*>(w);
                par.set_value(QString::number(nodeDlg->getNodeId()).toStdString());
                break;
            }
            case PluginParameter::ComboBox:
            {
                const QComboBox* cbox = static_cast<const QComboBox*>(w);
                par.set_value(cbox->currentText().toStdString());
                break;
            }
            default:
                continue;
                break;
            }
            settings.push_back(par);
        }
        QDialog::accept();
        mGuiExtensionInterface->set_parameter(settings);

        if (!buttonClicked.empty())
        {
            mGuiExtensionInterface->execute_function(buttonClicked,gNetlist,gSelectionRelay->selectedModulesVector(),gSelectionRelay->selectedGatesVector(),gSelectionRelay->selectedNetsVector());
            if (gPythonContext->pythonThread())
                gPythonContext->pythonThread()->unlock();
        }
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

        QString iconPath = (mParameter.get_type() == PluginParameter::NewFile)
                ? ":/icons/folder-down"
                : ":/icons/folder";
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
        QString filename;
        switch (mParameter.get_type()) {
        case PluginParameter::ExistingDir:
            filename = QFileDialog::getExistingDirectory(this,parLabel,dir);
            break;
        case PluginParameter::ExistingFile:
            filename = QFileDialog::getOpenFileName(this,parLabel,dir);
            break;
        case PluginParameter::NewFile:
            filename = QFileDialog::getSaveFileName(this,parLabel,dir);
            break;
        default:
            Q_ASSERT (1==0); // should never happen
        }
        if (!filename.isEmpty())
            mEditor->setText(filename);
    }

    QString PluginParameterFileDialog::getFilename() const
    {
        return mEditor->text();
    }

    PluginParameterNodeDialog::PluginParameterNodeDialog(const PluginParameter& par, QWidget* parent)
        : QWidget(parent), mParameter(par)
    {
        QGridLayout* layout = new QGridLayout(this);
        mNodeId = new QSpinBox(this);
        mNodeId->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        layout->addWidget(mNodeId,0,0);
        mNodeName = new QLabel(this);
        mNodeName->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mNodeName->setMinimumWidth(280);
        layout->addWidget(mNodeName,0,1);

        mButton = new QPushButton("",this);

        u32 defaultId = QString::fromStdString(mParameter.get_value()).toUInt();
        mNodeId->setValue(defaultId);

        QString iconPath;
        switch (mParameter.get_type()) {
        case PluginParameter::Module:
            iconPath = ":/icons/ne_module";
            setModule(defaultId);
            connect(mButton,&QPushButton::clicked,this,&PluginParameterNodeDialog::handleActivateModuleDialog);
            connect(mNodeId,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&PluginParameterNodeDialog::setModule);
            mValidIds = gNetlist->get_used_module_ids();
            break;
        case PluginParameter::Gate:
            iconPath = ":/icons/ne_gate";
            setGate(defaultId);
            connect(mButton,&QPushButton::clicked,this,&PluginParameterNodeDialog::handleActivateGateDialog);
            connect(mNodeId,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,&PluginParameterNodeDialog::setGate);
            mValidIds = gNetlist->get_used_gate_ids();
            break;
        default:
            Q_ASSERT(1==0); // widget must not be created if parameter type not module or gate
            break;
        }

        u32 maxValue = 0;
        for (u32 id : mValidIds)
            if (id > maxValue)
                maxValue = id;
        mNodeId->setMaximum(maxValue);
        if (defaultId > maxValue)
            mNodeId->setValue(0);
        mButton->setIcon(gui_utility::getStyledSvgIcon("all->#F0F0F1",iconPath));
        mButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
        layout->addWidget(mButton,0,2);
    }

    void PluginParameterNodeDialog::setGate(int id)
    {
        if (!id || !isValidId(id))
        {
            mNodeName->setText("(no gate selected)");
            return;
        }
        Gate* g = gNetlist->get_gate_by_id(id);
        if (g) mNodeName->setText(QString::fromStdString(g->get_name()));
    }

    void PluginParameterNodeDialog::setModule(int id)
    {
        if (!id || !isValidId(id))
        {
            mNodeName->setText("(no module selected)");
            return;
        }
        Module* m = gNetlist->get_module_by_id(id);
        if (m) mNodeName->setText(QString::fromStdString(m->get_name()));
    }

    bool PluginParameterNodeDialog::isValidId(int id) const
    {
        return (mValidIds.find(id)!=mValidIds.end());
    }

    void PluginParameterNodeDialog::handleActivateModuleDialog()
    {
        ModuleDialog md({}, "Select module", nullptr, this);
        if (md.exec() == QDialog::Accepted)
        {
            setModule(md.selectedId());
            mNodeId->setValue(md.selectedId());
        }
    }

    void PluginParameterNodeDialog::handleActivateGateDialog()
    {
        GateDialog gd({}, "Select gate", nullptr, this);
        if (gd.exec() == QDialog::Accepted)
        {
            setGate(gd.selectedId());
            mNodeId->setValue(gd.selectedId());
        }
    }

    int PluginParameterNodeDialog::getNodeId() const
    {
        return mNodeId->value();
    }
}
