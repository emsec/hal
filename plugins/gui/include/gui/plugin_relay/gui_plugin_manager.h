// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <QObject>
#include <QMap>
#include <QLabel>
#include <QIcon>
#include <QStringList>
#include <QDateTime>
#include <QFileInfo>
#include <QVariant>
#include <QAbstractTableModel>
#include <QTableView>
#include <QItemDelegate>
#include <QMenu>

#include "gui/gui_utils/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"
#include "hal_core/utilities/program_options.h"

class QSettings;
class QPushButton;

namespace hal {
    class GuiExtensionInterface;

    class GuiPluginEntry
    {
    public:
        friend class GuiPluginTable;
        enum LoadState { NotAPlugin, NotLoaded, AutoLoad, UserLoad } mLoadState;
        QString mName;
        QString mVersion;
        QString mDescription;
        QString mFilePath;
        QString mExternalPath;
        QDateTime mFileModified;
        QStringList mDependencies;
        FacExtensionInterface::Feature mFeature;
        QStringList mFeatureArguments;
        bool mUserInterface;
        enum  GuiExtensionState { Unknown, NotAnExtension, Disabled, Enabled} mGuiExtensionState;
        QString mCliOptions;
        bool mFileFound;

    public:
        GuiPluginEntry(const QFileInfo& info);
        GuiPluginEntry(const QSettings* settings);
        QVariant data(int icol) const;
        QString name() const { return mName; }
        void persist(QSettings* settings) const;
        bool requestLoad() const { return mLoadState == UserLoad; }
        bool isLoaded() const { return mLoadState == AutoLoad || mLoadState == UserLoad; }
        bool isPlugin() const { return mLoadState != NotAPlugin; }
        void updateFromLoaded(const BasePluginInterface* bpif, bool isUser, const QDateTime& modified = QDateTime());
        bool isFileFound() const { return mFileFound; }
        void setFileFound(bool fnd) { mFileFound = fnd; }
        GuiExtensionState enforceGuiExtensionState(GuiExtensionInterface* geif) const;
    };

    class GuiPluginManager;

    class GuiPluginDelegate : public QItemDelegate
    {
        Q_OBJECT
        friend class GuiPluginManager;
        QIcon mIconLoad;
        QIcon mIconUnload;
        QIcon mIconEnableGuiContribution;
        QIcon mIconDisableGuiContribution;
        QIcon mIconCliOptions;
        QPoint mMousePos;
        QModelIndex mMouseIndex;
        QPushButton* mTemplateButton[3];
        enum State {Normal, Hover, Pressed};
    Q_SIGNALS:
        void buttonPressed(QModelIndex index);
    public:
        GuiPluginDelegate(QObject* parent = nullptr);
        ~GuiPluginDelegate();
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void updateQss(GuiPluginManager* gpm);
    };

    class SupportedFileFormats : public QMap<QString,QString>
    {
        FacExtensionInterface::Feature mFeature;
    public:
        SupportedFileFormats(FacExtensionInterface::Feature ft) : mFeature(ft) {;}
        QString toFileDialog(bool addHalFormat) const;
    };

    class GuiPluginTable : public QAbstractTableModel
    {
        Q_OBJECT
        QList<GuiPluginEntry*> mEntries;
        QList<GuiPluginEntry*> mAvoid;
        QMap<QString,int> mLookup;
        QSettings* mSettings;
        bool mWaitForRefresh;
        GuiPluginManager* mPluginMgr;

        void changeState(const QString& pluginName, GuiPluginEntry::LoadState state);
        void populateTable(bool refresh);
        void clearMemory();
    Q_SIGNALS:
        void toggleEnableGuiContribution(QString pluginName);
        void showCliOptions(QString pluginName, QString cliOptions);
    public Q_SLOTS:
        void handleButtonPressed(const QModelIndex& buttonIndex);
        void handlePluginLoaded(const QString& pluginName, const QString& path);
        void handlePluginUnloaded(const QString& pluginName, const QString& path);
        void handleRefresh();
    public:
        GuiPluginTable(GuiPluginManager* parent = nullptr);
        ~GuiPluginTable();

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        BasePluginInterface* load(const QString& pluginName, const QString& path) const;
        QStringList neededBy(const QString& pluginName);
        void persist();
        bool isLoaded(const QModelIndex& index) const;
        GuiPluginEntry::GuiExtensionState guiExtensionState(const QModelIndex& index) const;
        void setGuiExtensionState(const QString& pluginName, GuiPluginEntry::GuiExtensionState state);
        bool hasCliExtension(const QModelIndex& index) const;
        bool isHalGui(const QModelIndex& index) const;
        void loadFeature(FacExtensionInterface::Feature ft, const QString& extension=QString());
        SupportedFileFormats listFacFeature(FacExtensionInterface::Feature ft) const;
        GuiPluginEntry* at(int irow) const;
        int addExternalPlugin(const QString& path);
        void removeEntry(int irow);
   };

    class GuiPluginView : public QTableView
    {
        Q_OBJECT
    public:
        GuiPluginView(QWidget* parent = nullptr);
    };

    class GuiPluginManager : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString loadIconPath READ loadIconPath WRITE setLoadIconPath)
        Q_PROPERTY(QString loadIconStyle READ loadIconStyle WRITE setLoadIconStyle)
        Q_PROPERTY(QString unloadIconPath READ unloadIconPath WRITE setUnloadIconPath)
        Q_PROPERTY(QString unloadIconStyle READ unloadIconStyle WRITE setUnloadIconStyle)
        Q_PROPERTY(QString cliIconPath READ cliIconPath WRITE setCliIconPath)
        Q_PROPERTY(QString cliIconStyle READ cliIconStyle WRITE setCliIconStyle)
        Q_PROPERTY(QString guiIconEnabledPath READ guiIconEnabledPath WRITE setGuiIconEnabledPath)
        Q_PROPERTY(QString guiIconDisabledPath READ guiIconDisabledPath WRITE setGuiIconDisabledPath)
        Q_PROPERTY(QString guiIconEnabledStyle READ guiIconEnabledStyle WRITE setGuiIconEnabledStyle)
        Q_PROPERTY(QString guiIconDisabledStyle READ guiIconDisabledStyle WRITE setGuiIconDisabledStyle)
        Q_PROPERTY(QColor defaultTextColor READ defaultTextColor WRITE setDefaultTextColor)
        Q_PROPERTY(QColor hilightTextColor READ hilightTextColor WRITE setHilightTextColor)
        Q_PROPERTY(QColor hilightBackgroundColor READ hilightBackgroundColor WRITE setHilightBackgroundColor)

        GuiPluginView* mGuiPluginView;
        GuiPluginTable* mGuiPluginTable;
        GuiPluginDelegate* mGuiPluginDelegate;
        QString mLoadIconPath;
        QString mLoadIconStyle;
        QString mUnloadIconPath;
        QString mUnloadIconStyle;
        QString mCliIconPath;
        QString mCliIconStyle;
        QString mGuiIconEnabledPath;
        QString mGuiIconDisabledPath;
        QString mGuiIconEnabledStyle;
        QString mGuiIconDisabledStyle;
        QColor mDefaultTextColor;
        QColor mHilightTextColor;
        QColor mHilightBackgroundColor;
        QLabel* mIconLegend[4];
    Q_SIGNALS:
        void backToNetlist();
    private Q_SLOTS:
        void handleToggleGuiContribution(const QString& pluginName);
        void handleShowCliOptions(const QString& pluginName, const QString& cliOptions);
    public Q_SLOTS:
        void handleButtonCancel();
        void handleLoadExternalPlugin();
    public:
        GuiPluginManager(QWidget* parent = nullptr);
        static QMap<QString,GuiExtensionInterface*> getGuiExtensions();
        /**
         * Adds context menu actions, which are provided by currently loaded plugins, 
         * for specified set of netlist elements to a QMenu.
         * 
         * @param contextMenu - The QMenu to which the actions are added to.
         * @param netlist - The netlist associated with the netlist elements.
         * @param modules - List with ids of modules on which the actions will operate.
         * @param gates - List with ids of gates on which the actions will operate.
         * @param nets - List with ids of nets on which the actions will operate.
         */
        static void addPluginSubmenus(QMenu* contextMenu, Netlist* netlist,
                                      const std::vector<u32>& modules,
                                      const std::vector<u32>& gates,
                                      const std::vector<u32>& nets);
        void repolish();

        QString loadIconPath() const;
        QString loadIconStyle() const;
        QString unloadIconPath() const;
        QString unloadIconStyle() const;
        QString cliIconPath() const;
        QString cliIconStyle() const;
        QString guiIconEnabledPath() const;
        QString guiIconDisabledPath() const;
        QString guiIconEnabledStyle() const;
        QString guiIconDisabledStyle() const;
        QColor defaultTextColor() const;
        QColor hilightTextColor() const;
        QColor hilightBackgroundColor() const;

        void setLoadIconPath(const QString& s);
        void setLoadIconStyle(const QString& s);
        void setUnloadIconPath(const QString& s);
        void setUnloadIconStyle(const QString& s);
        void setCliIconPath(const QString& s);
        void setCliIconStyle(const QString& s);
        void setGuiIconEnabledPath(const QString& s);
        void setGuiIconDisabledPath(const QString& s);
        void setGuiIconEnabledStyle(const QString& s);
        void setGuiIconDisabledStyle(const QString& s);
        void setDefaultTextColor(QColor& c);
        void setHilightTextColor(QColor& c);
        void setHilightBackgroundColor(QColor& c);

        void addPluginActions(QMenu* menu) const;
    };
}
