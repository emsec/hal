#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/gui_globals.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/cli_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include <QDebug>
#include <QMap>
#include <QDir>
#include <iostream>
#include <filesystem>
#include <QDateTime>
#include <QSettings>
#include <QBrush>
#include <QApplication>
#include <QPainter>
#include <QPushButton>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

namespace hal {


    GuiPluginManager::GuiPluginManager(QWidget *parent)
        : QWidget(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);

        mGuiPluginView = new GuiPluginView(this);
        mGuiPluginTable = new GuiPluginTable(this);
        connect(mGuiPluginTable,&GuiPluginTable::triggerInvokeGui,this,&GuiPluginManager::handleInvokeGui);
        connect(mGuiPluginTable,&GuiPluginTable::showCliOptions,this,&GuiPluginManager::handleShowCliOptions);
        connect(gPluginRelay,&PluginRelay::pluginLoaded,mGuiPluginTable,&GuiPluginTable::handlePluginLoaded);
        connect(gPluginRelay,&PluginRelay::pluginUnloaded,mGuiPluginTable,&GuiPluginTable::handlePluginUnloaded);
        gPluginRelay->mGuiPluginTable = mGuiPluginTable;
        mGuiPluginView->setModel(mGuiPluginTable);

        GuiPluginDelegate* delegate = new GuiPluginDelegate(this);
        connect(delegate,&GuiPluginDelegate::buttonPressed,mGuiPluginTable,&GuiPluginTable::handleButtonPressed);
        mGuiPluginView->setItemDelegateForColumn(10,delegate);
        mGuiPluginView->setItemDelegateForColumn(8,delegate);
        mGuiPluginView->setItemDelegateForColumn(7,delegate);
        mGuiPluginView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        mGuiPluginView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        mGuiPluginView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        layout->addWidget(mGuiPluginView,Qt::AlignLeft);

        QLabel* legendHeader = new QLabel("Buttons used in table above:", this);
        legendHeader->setFixedHeight(48);
        layout->addWidget(legendHeader,Qt::AlignLeft);
        QPixmap* picon[4] = {
            new QPixmap(":/icons/insert_plugin", "PNG"),
            new QPixmap(":/icons/x_delete",      "PNG"),
            new QPixmap(":/icons/cli_options",   "PNG"),
            new QPixmap(":/icons/invoke_gui",    "PNG")};
        QLabel* ltext[4] = {
            new QLabel("Load HAL plugin and dependencies (if any)", this),
            new QLabel("Unload HAL plugin unless needed as dependency by other plugin", this),
            new QLabel("This plugin contributes CLI options. Click button for description of options.", this),
            new QLabel("This plugin contributes its own settings menu to GUI. Click button in order to return to netlist and open menu.", this)};

        for (int i=0; i<4; i++)
        {
            QHBoxLayout* legend = new QHBoxLayout;
            QLabel* licon = new QLabel(this);
            licon->setPixmap(picon[i]->scaled(32,32));
            licon->setAlignment(Qt::AlignCenter);
            licon->setFixedSize(52,52);
            legend->addWidget(licon);
            ltext[i]->setIndent(8);
            legend->addWidget(ltext[i]);
            layout->addLayout(legend);
        }

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setAlignment(Qt::AlignRight);
        QPushButton* butCancel  = new QPushButton("Cancel", this);
        connect(butCancel,&QPushButton::clicked,this,&GuiPluginManager::handleButtonCancel);
        buttonLayout->addWidget(butCancel);
        QPushButton* butRefresh = new QPushButton("Refresh", this);
        connect(butRefresh,&QPushButton::clicked,mGuiPluginTable,&GuiPluginTable::handleRefresh);
        buttonLayout->addWidget(butRefresh);
        layout->addLayout(buttonLayout);
    }

    void GuiPluginManager::handleButtonCancel()
    {
        Q_EMIT backToNetlist(QString());
    }

    void GuiPluginManager::handleInvokeGui(const QString &pluginName)
    {
        Q_EMIT backToNetlist(pluginName);
    }

    void GuiPluginManager::handleShowCliOptions(const QString &pluginName, const QString &cliOptions)
    {
        QMessageBox* msg = new QMessageBox(QMessageBox::NoIcon, "CLI Options for "+pluginName, cliOptions);
        msg->setStyleSheet("QLabel{min-width: 800px;}");
        msg->exec();
    }

    //_________________VIEW_______________________________
    GuiPluginView::GuiPluginView(QWidget* parent)
        : QTableView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }

    //_________________TABLE______________________________
    GuiPluginTable::GuiPluginTable(QObject* parent)
        : QAbstractTableModel(parent)
    {
        mWaitForRefresh = true;
        populateTable(false);
        mWaitForRefresh = false;
    }

    GuiPluginTable::~GuiPluginTable()
    {
        clearMemory();
    }

    void GuiPluginTable::handleRefresh()
    {
        if (mWaitForRefresh) return;

        beginResetModel();
        mWaitForRefresh = true;
        clearMemory();
        populateTable(true);
        mWaitForRefresh = false;
        endResetModel();
    }

    void GuiPluginTable::populateTable(bool refresh)
    {
        QDir userConfigDir(QString::fromStdString(utils::get_user_config_directory()));
        QMap<QString,GuiPluginEntry*> pluginEntries;
        mSettings = new QSettings(userConfigDir.absoluteFilePath("plugins.ini"),QSettings::IniFormat);

        std::set<std::string> loadedPlugins = plugin_manager::get_plugin_names();

        int nentry = mSettings->beginReadArray("plugins");
        for (int i=0; i<nentry; i++)
        {
            mSettings->setArrayIndex(i);
            GuiPluginEntry* gpe = new GuiPluginEntry(mSettings);
            pluginEntries.insert(gpe->mName,gpe);
        }
        mSettings->endArray();

        for (std::filesystem::path plugDir : utils::get_plugin_directories())
        {
            QDir dir(QString::fromStdString(plugDir.string()));
            if (!dir.exists()) continue;
            for (QFileInfo info : dir.entryInfoList())
            {
                if (!info.isFile()) continue;
                if (!plugin_manager::has_valid_file_extension(info.absoluteFilePath().toStdString())) continue;

                QString pluginName = info.baseName();
                GuiPluginEntry* gpe = pluginEntries.value(pluginName);
                bool needUpdate = true;
                if (gpe)
                {
                   needUpdate = gpe->mFileModified != info.lastModified();
                }
                else
                {
                    gpe = new GuiPluginEntry(info);
                    pluginEntries.insert(pluginName,gpe);
                }

                BasePluginInterface* bpif = plugin_manager::get_plugin_instance(pluginName.toStdString(), true, true);
                if (bpif && !refresh)
                {
                    // plugin already loaded ?
                    bool alreadyLoaded = loadedPlugins.find(pluginName.toStdString())!=loadedPlugins.end();
                    gpe->updateFromLoaded(bpif, alreadyLoaded);
                    qDebug() << pluginName << "already loaded" << alreadyLoaded;
                    continue;
                }
                else if (gpe->requestLoad())
                {
                    // user requested this plugin explicitly in previous session
                    bpif = load(pluginName, info.absoluteFilePath());
                    if (!bpif)
                    {
                        gpe->mState = GuiPluginEntry::NotAPlugin; // however, load failed
                        qDebug() << pluginName << "load failed";
                    }
                    else
                    {
                        gpe->updateFromLoaded(bpif, true, info.lastModified());
                        qDebug() << pluginName << "load upon user request";
                    }
                    continue;
                }

                if (!needUpdate)
                {
                    qDebug() << pluginName << "don't need update" << gpe->mState;
                    // known plugin, load only when needed
                    if (gpe->isPlugin())
                        gpe->mState = GuiPluginEntry::NotLoaded;
                    continue;
                }

                // plugin updated or not known yet, try dummy load to retrieve information
                bpif = load(pluginName, info.absoluteFilePath());
                if (!bpif)
                {
                    qDebug() << pluginName << "dummy load failed" << info.absoluteFilePath();
                    gpe->mState = GuiPluginEntry::NotAPlugin;
                    continue;
                }

                if (bpif->get_name().empty() || bpif->get_version().empty())
                {
                    qDebug() << pluginName << "name or version empty, discarded";
                    gpe->mState = GuiPluginEntry::NotAPlugin;
                }
                else
                {
                    qDebug() << pluginName << "dummy load success";
                    gpe->updateFromLoaded(bpif, false, info.lastModified());    // load success
                }
            }
        }

        // update entry if plugin loaded in meantime as dependency
        for (std::string loadedPluginName:  plugin_manager::get_plugin_names())
        {
            GuiPluginEntry* gpe = pluginEntries.value(QString::fromStdString(loadedPluginName));
            if (!gpe || gpe->isLoaded()) continue;
            BasePluginInterface* bpif = plugin_manager::get_plugin_instance(loadedPluginName,false,true);
            if (!bpif) continue;
            QFileInfo info(gpe->mFilePath);
            gpe->updateFromLoaded(bpif,false,info.exists()?info.lastModified():QDateTime());
        }

        mEntries.clear();
        mAvoid.clear();
        mLookup.clear();
        for (auto it = pluginEntries.constBegin(); it != pluginEntries.constEnd(); ++it)
        {
            GuiPluginEntry* gpe = it.value();
            if (gpe->isPlugin())
            {
                mLookup.insert(gpe->mName,mEntries.size());
                mEntries.append(gpe);
            }
            else
            {
                mAvoid.append(gpe);
            }
        }
        persist();
    }

    void GuiPluginTable::handlePluginLoaded(const QString& pluginName, const QString&)
    {
        if (mWaitForRefresh) return;
        changeState(pluginName,GuiPluginEntry::AutoLoad);
    }

    void GuiPluginTable::handlePluginUnloaded(const QString& pluginName, const QString&)
    {
        if (mWaitForRefresh) return;
        changeState(pluginName,GuiPluginEntry::NotLoaded);
    }

    void GuiPluginTable::changeState(const QString& pluginName, GuiPluginEntry::State state)
    {
        int irow = mLookup.value(pluginName,-1);
        if (irow < 0 && irow >= mEntries.size()) return;
        mEntries.at(irow)->mState = state;
        Q_EMIT dataChanged(index(irow,0),index(irow,10));
    }

    void GuiPluginTable::loadFeature(FacExtensionInterface::Feature ft, const QString& extension)
    {
        for (GuiPluginEntry* gpe : mEntries)
        {
            if (gpe->isLoaded() || gpe->mFeature != ft) continue;
            if (!extension.isEmpty())
            {
                bool extensionFound = false;
                for (QString arg : gpe->mFeatureArguments)
                {
                    if (arg==extension)
                    {
                        extensionFound = true;
                        break;
                    }
                }
                if (!extensionFound) continue;
            }
            BasePluginInterface* bpif = load(gpe->mName, gpe->mFilePath);
            if (bpif) gpe->updateFromLoaded(bpif,false);
        }
    }

    BasePluginInterface* GuiPluginTable::load(const QString& pluginName, const QString& path) const
    {        
        std::string pnam = pluginName.toStdString();
        if (!plugin_manager::load(pnam,path.toStdString()))
        {
            log_warning("gui", "Error loading plugin '{}' from location '{}'",
                        pnam,path.toStdString());
            return nullptr;
        }
        return plugin_manager::get_plugin_instance(pnam);
    }

    void GuiPluginTable::persist()
    {
        mSettings->clear();
        mSettings->beginWriteArray("plugins");
        int inx = 0;

        for (GuiPluginEntry* gpe : mEntries)
        {
            mSettings->setArrayIndex(inx++);
            gpe->persist(mSettings);
        }
        for (GuiPluginEntry* gpe : mAvoid)
        {
            mSettings->setArrayIndex(inx++);
            gpe->persist(mSettings);
        }
        mSettings->endArray();
        mSettings->sync();
    }

    QMap<QString,GuiExtensionInterface*> GuiPluginManager::getGuiExtensions()
    {
        QMap<QString,GuiExtensionInterface*> retval;
        for (const std::string& pluginName : plugin_manager::get_plugin_names())
        {
            BasePluginInterface* bpif = plugin_manager::get_plugin_instance(pluginName);
            if (!bpif) continue;
            for (AbstractExtensionInterface* aeif : bpif->get_extensions())
            {
                GuiExtensionInterface* geif = dynamic_cast<GuiExtensionInterface*>(aeif);
                if (geif) retval.insert(QString::fromStdString(pluginName),geif);
            }
        }
        return retval;
    }

    void GuiPluginTable::clearMemory()
    {

        for (GuiPluginEntry* entry : mEntries)
            delete entry;

        mEntries.clear();
        mAvoid.clear();
        mLookup.clear();
        mSettings->deleteLater();
        mSettings = nullptr;
    }

    int GuiPluginTable::rowCount(const QModelIndex&) const
    {
        return mEntries.size();
    }

    int GuiPluginTable::columnCount(const QModelIndex&) const
    {
        return 11;
    }

    QVariant GuiPluginTable::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case 0: return "Name";
            case 1: return "Description";
            case 2: return "Filename";
            case 3: return "Timestamp";
            case 4: return "Dependencies";
            case 5: return "Category";
            case 6: return "Extensions";
            case 7: return "GUI";
            case 8: return "CLI";
            case 9: return "State";
            case 10: return QString();
            default: return QVariant();
            }
        }
        return QAbstractTableModel::headerData(section,orientation,role);
    }

    void GuiPluginTable::handleButtonPressed(const QModelIndex& buttonIndex)
    {
        if (mWaitForRefresh) return;

        int irow = buttonIndex.row();
        if (irow >= mEntries.size()) return;
        GuiPluginEntry* gpe = mEntries.at(irow);
        switch (buttonIndex.column())
        {
        case 7:
            if (gpe->mName == "hal_gui") return;
            if (gpe->isLoaded())
                Q_EMIT triggerInvokeGui(gpe->mName);
            return;
        case 8:
            if (!gpe->mCliOptions.isEmpty())
                Q_EMIT showCliOptions(gpe->mName, gpe->mCliOptions);
            return;
        }
        if (gpe->mName == "hal_gui") return;

        bool success = false;
        if (gpe->isLoaded())
        {
            QStringList need = neededBy(gpe->mName);
            if (need.isEmpty())
            {
                success = plugin_manager::unload(gpe->mName.toStdString());
                if (success)
                    gpe->mState = GuiPluginEntry::NotLoaded;
                else
                    QMessageBox::warning(qApp->activeWindow(),"Unload failed",
                                         QString("Unload of plugin %1\nrefused by plugin_manager")
                                         .arg(gpe->mName));
            }
            else
            {
                QMessageBox::warning(qApp->activeWindow(),"Unload failed",
                                     QString("Cannot unload plugin %1\nwhich is required by %2")
                                     .arg(gpe->mName).arg(need.join(',')));
            }
        }
        else
        {
            if (plugin_manager::load(gpe->mName.toStdString(),gpe->mFilePath.toStdString()))
            {
                success = true;
                gpe->mState = GuiPluginEntry::UserLoad;
            }
            else
                QMessageBox::warning(qApp->activeWindow(),"Load failed",
                                     QString("Could not load plugin %1\nplease check plugin_manager log")
                                     .arg(gpe->mName));
        }
        if (success)
        {
            Q_EMIT dataChanged(index(irow,0),index(irow,columnCount()-1));
            persist();
        }
    }

    QVariant GuiPluginTable::data(const QModelIndex &index, int role) const
    {
        if (role == Qt::ForegroundRole)
            return QBrush(mEntries.at(index.row())->isLoaded() ? Qt::yellow : Qt::lightGray);
        if (role != Qt::DisplayRole) return QVariant();
        QVariant v = mEntries.at(index.row())->data(index.column());
        if (index.column() >= 7 && index.column() <= 8)
            return (v.toBool() ? "X" : " ");
        return v;
    }

    bool GuiPluginTable::isHalGui(const QModelIndex &index) const
    {
        if (index.row() >= mEntries.size()) return false;
        return mEntries.at(index.row())->mName == "hal_gui";
    }

    bool GuiPluginTable::hasGuiExtension(const QModelIndex& index) const
    {
        if (index.row() >= mEntries.size()) return false;
        return mEntries.at(index.row())->mGuiExtensions;
    }

    bool GuiPluginTable::hasCliExtension(const QModelIndex& index) const
    {
        if (index.row() >= mEntries.size()) return false;
        return !mEntries.at(index.row())->mCliOptions.isEmpty();
    }

    bool GuiPluginTable::isLoaded(const QModelIndex &index) const
    {
        if (index.row() >= mEntries.size()) return false;
        return mEntries.at(index.row())->isLoaded();
    }

    QStringList GuiPluginTable::neededBy(const QString &pluginName)
    {
        QStringList retval;
        for (GuiPluginEntry* gpe : mEntries)
        {
            if (gpe->mName == pluginName) continue;
            for (QString dep : gpe->mDependencies)
                if (dep == pluginName)
                {
                    // dependency found, check if loaded
                    if (gpe->isLoaded())
                        retval.append(gpe->mName);
                }
        }
        return retval;
    }

    //------------------------------------------------------------

    GuiPluginEntry::GuiPluginEntry(const QFileInfo& info)
        : mState(NotAPlugin), mName(info.baseName()),
          mFilePath(info.absoluteFilePath()),
          mFileModified(info.lastModified()),
          mFeature(FacExtensionInterface::FacUnknown),
          mUserInterface(false),
          mGuiExtensions(false)
    {;}

    QVariant GuiPluginEntry::data(int icol) const
    {
        switch (icol)
        {
        case 0: return mName;
        case 1: return mDescription;
        case 2: return QFileInfo(mFilePath).fileName();
        case 3: return mFileModified.toString("dd.MM.yy hh:mm");
        case 4: return mDependencies.join(' ');
        case 5:
        {
            switch (mFeature)
            {
            case FacExtensionInterface::FacNetlistParser: return "Netlist parser";
            case FacExtensionInterface::FacNetlistWriter: return "Netlist writer";
            case FacExtensionInterface::FacGatelibParser: return "Gate library parser";
            case FacExtensionInterface::FacGatelibWriter: return "Gate library writer";
            default: break;
            }
            if (mUserInterface) return "HAL user interface";
            return "Other HAL plugin";
        }
        case 6: return mFeatureArguments.join(' ');
        case 7: return mGuiExtensions;
        case 8: return mCliOptions;
        case 9: return isLoaded() ? "LOADED" : "-";
        case 10: return (int) mState;
        }
        return QVariant();
    }

    void GuiPluginEntry::persist(QSettings* settings) const
    {
        settings->setValue("state", (int) mState);
        settings->setValue("name", mName);
        settings->setValue("version", mVersion);
        settings->setValue("description", mDescription);
        settings->setValue("file_path", mFilePath);
        settings->setValue("file_modified", mFileModified);
        settings->setValue("dependencies", mDependencies);
        settings->setValue("feature_code", (int) mFeature);
        settings->setValue("feature_args", mFeatureArguments);
        settings->setValue("user_interface", mUserInterface);
        settings->setValue("extends_gui", mGuiExtensions);
        settings->setValue("cli_options", mCliOptions);
    }

    GuiPluginEntry::GuiPluginEntry(const QSettings *settings)
    {
        mState    = (State) settings->value("state").toInt();
        mName             = settings->value("name").toString();
        mVersion          = settings->value("version").toString();
        mDescription      = settings->value("description").toString();
        mFilePath         = settings->value("file_path").toString();
        mFileModified     = settings->value("file_modified").toDateTime();
        mDependencies     = settings->value("dependencies").toStringList();
        mFeature = (FacExtensionInterface::Feature) settings->value("feature_code").toInt();
        mFeatureArguments = settings->value("feature_args").toStringList();
        mUserInterface    = settings->value("user_interface").toBool();
        mGuiExtensions    = settings->value("extends_gui").toBool();
        mCliOptions    = settings->value("cli_options").toString();
    }

    void GuiPluginEntry::updateFromLoaded(const BasePluginInterface *bpif, bool isUser, const QDateTime& modified)
    {
        mState   = isUser ? UserLoad : AutoLoad;
        QString desc = QString::fromStdString(bpif->get_name());
        if (desc != mName) mDescription = desc;
        if (!bpif->get_description().empty()) mDescription = QString::fromStdString(bpif->get_description());
        mVersion = QString::fromStdString(bpif->get_version());
        mDependencies.clear();
        for (std::string dep : bpif->get_dependencies())
             mDependencies.append(QString::fromStdString(dep));
        mFeatureArguments.clear();
        for (plugin_manager::PluginFeature feature : plugin_manager::get_plugin_features(bpif->get_name()))
        {
            mDescription = QString::fromStdString(feature.description);
            mFeature = feature.feature;
            mFeatureArguments.clear();
            for (std::string arg : feature.args)
                mFeatureArguments.append(QString::fromStdString(arg));
        }
        mCliOptions.clear();
        for (AbstractExtensionInterface* aeif : bpif->get_extensions())
        {
            CliExtensionInterface* ceif = nullptr;
            if (dynamic_cast<GuiExtensionInterface*>(aeif))
                mGuiExtensions = true;
            else if ((ceif=dynamic_cast<CliExtensionInterface*>(aeif)))
                mCliOptions = QString::fromStdString(ceif->get_cli_options().get_options_string());
        }
        mUserInterface = (dynamic_cast<const UIPluginInterface*>(bpif) != nullptr);
        if (modified.isValid()) mFileModified = modified;
    }

    //------------------------------------------------
    GuiPluginDelegate::GuiPluginDelegate(QObject *parent)
        : QItemDelegate(parent),
          mIconLoad(":/icons/insert_plugin"),
          mIconUnload(":/icons/x_delete"),
          mIconInvokeGui(":/icons/invoke_gui"),
          mIconDisabledGui(":/icons/disabled_gui"),
          mIconCliOptions(":/icons/cli_options")
    {
        int bgBright[3] = {26, 46, 80};
        for (int i=0; i<3; i++)
        {
            mTemplateButton[i] = new QPushButton;
            mTemplateButton[i]->setStyleSheet(QString("background-color : rgb(%1,%1,%1)").arg(bgBright[i]));
        }
    }

    GuiPluginDelegate::~GuiPluginDelegate()
    {
        for (int i=0; i<3; i++)
            delete mTemplateButton[i];
    }

    void GuiPluginDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        const int w = 28;
        const GuiPluginTable* plt = dynamic_cast<const GuiPluginTable*>(index.model());
        if (!plt) return;
        QStyleOptionButton button;
        switch (index.column())
        {
        case 7:
            button.iconSize = QSize(w-4,w-4);
            if (plt->hasGuiExtension(index))
                button.icon = (plt->isLoaded(index) && gNetlist) ? mIconInvokeGui : mIconDisabledGui;
            else return;
            break;
        case 8:
            button.iconSize = QSize(w-4,w-4);
            if (plt->hasCliExtension(index))
                button.icon = mIconCliOptions;
            else return;
            break;
        case 10:
            if (plt->isHalGui(index)) return;
            if (plt->isLoaded(index))
                button.icon = mIconUnload;
            else
                button.icon = mIconLoad;
            button.iconSize = QSize(w-12,w-12);
            break;
        default:
            return;
        }

        QRect r = option.rect;
        int x = r.left() + r.width() - w;
        int y = r.top();
        int h = w;
        State stat = Normal;
        QColor bgcol[3] = { Qt::black, Qt::gray, Qt::red};
        if (mMouseIndex.isValid() && mMouseIndex == index) stat = Pressed;
        button.rect = QRect(x,y,w,h);
        button.state = (stat!=Pressed) ? QStyle::State_Enabled | QStyle::State_Raised : QStyle::State_Enabled | QStyle::State_Sunken | QStyle::State_MouseOver;
        painter->fillRect(r,QBrush(bgcol[stat]));
        QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter, mTemplateButton[stat]);
    }

    QSize GuiPluginDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return QSize(30,30);
    }

    bool GuiPluginDelegate::editorEvent(QEvent* event, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex& index)
    {
        QMouseEvent* mev = dynamic_cast<QMouseEvent*>(event);
        if (!mev) return false;
        switch (mev->type())
        {
        case QEvent::MouseButtonPress:
            mMousePos = mev->pos();
            mMouseIndex = index;
            return true;
        case QEvent::MouseButtonRelease:
            if ((mMousePos - mev->pos()).manhattanLength()<4)
            {
                Q_EMIT buttonPressed(index);
            }
            mMousePos = QPoint();
            mMouseIndex = QModelIndex();
            return true;
        default:
            break;
        }
        return false;
    }
}
