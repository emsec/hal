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
#include <QIcon>
#include <QStringList>
#include <QDateTime>
#include <QFileInfo>
#include <QVariant>
#include <QAbstractTableModel>
#include <QTableView>
#include <QItemDelegate>
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"

class QSettings;
class QPushButton;

namespace hal {
    class GuiExtensionInterface;

    class GuiPluginEntry
    {
    public:
        enum State { NotAPlugin, NotLoaded, AutoLoad, UserLoad };
        friend class GuiPluginTable;
        State mState;
        QString mName;
        QString mVersion;
        QString mDescription;
        QString mFilePath;
        QDateTime mFileModified;
        QStringList mDependencies;
        FacExtensionInterface::Feature mFeature;
        QStringList mFeatureArguments;
        bool mGuiExtensions;
        bool mCliExtensions;

    public:
        GuiPluginEntry(const QFileInfo& info);
        GuiPluginEntry(const QSettings* settings);
        QVariant data(int icol) const;
        void persist(QSettings* settings) const;
        bool requestLoad() const { return mState == UserLoad; }
        bool isLoaded() const { return mState == AutoLoad || mState == UserLoad; }
        bool isPlugin() const { return mState != NotAPlugin; }
        void updateFromLoaded(const BasePluginInterface* bpif, bool isUser, const QDateTime& modified = QDateTime());
    };

    class GuiPluginDelegate : public QItemDelegate
    {
        Q_OBJECT
        friend class GuiPluginManager;
        QIcon mIconLoad;
        QIcon mIconUnload;
        QIcon mIconInvokeGui;
        QIcon mIconDisabledGui;
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
    };

    class GuiPluginTable : public QAbstractTableModel
    {
        Q_OBJECT
        QList<GuiPluginEntry*> mEntries;
        QList<GuiPluginEntry*> mAvoid;
        QMap<QString,int> mLookup;
        QSettings* mSettings;

        void changeState(const QString& pluginName, GuiPluginEntry::State state);
    Q_SIGNALS:
        void triggerInvokeGui(QString pluginName);
    public Q_SLOTS:
        void handleButtonPressed(const QModelIndex& buttonIndex);
        void handlePluginLoaded(const QString& pluginName, const QString& path);
        void handlePluginUnloaded(const QString& pluginName, const QString& path);
    public:
        GuiPluginTable(QObject* parent = nullptr);
        ~GuiPluginTable();

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        BasePluginInterface* load(const QString& pluginName, const QString& path) const;
        QStringList neededBy(const QString& pluginName);
        void persist();
        bool isLoaded(const QModelIndex& index) const;
        bool hasGuiExtension(const QModelIndex& index) const;
        bool isHalGui(const QModelIndex& index) const;
        void loadFeature(FacExtensionInterface::Feature ft, const QString& extension=QString());
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
        GuiPluginView* mGuiPluginView;
        GuiPluginTable* mGuiPluginTable;
    Q_SIGNALS:
        void backToNetlist(QString invokeGui);
    private Q_SLOTS:
        void handleButtonCancel();
        void handleButtonRefresh();
        void handleInvokeGui(const QString& pluginName);
    public:
        GuiPluginManager(QWidget* parent = nullptr);
        static QMap<QString,GuiExtensionInterface*> getGuiExtensions();
    };
}
