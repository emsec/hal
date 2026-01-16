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
#include "gui/action/action.h"
#include <QString>
#include <QWidget>

namespace hal {
    class MainWindow;
    class GateLibraryManager;
    class GuiPluginManager;
    class SettingsItemKeybind;

    class FileActions : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveAsIconPath READ saveAsIconPath WRITE setSaveAsIconPath)
        Q_PROPERTY(QString enabledIconStyle READ enabledIconStyle WRITE setEnabledIconStyle)
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString closeIconPath READ closeIconPath WRITE setCloseIconPath)
        Q_PROPERTY(QString closeIconStyle READ closeIconStyle WRITE setCloseIconStyle)
        Q_PROPERTY(QString newFileIconPath READ newFileIconPath WRITE setNewFileIconPath)
        Q_PROPERTY(QString newFileIconStyle READ newFileIconStyle WRITE setNewFileIconStyle)
        Q_PROPERTY(QString openProjIconPath READ openProjIconPath WRITE setOpenProjIconPath)
        Q_PROPERTY(QString openProjIconStyle READ openProjIconStyle WRITE setOpenProjIconStyle)

        MainWindow* mMainWindowReference;
        GateLibraryManager* mGatelibReference;
        GuiPluginManager* mGuiPluginReference;

        Action* mActionCreate;
        Action* mActionOpen;
        Action* mActionClose;
        Action* mActionSave;
        Action* mActionSaveAs;

        QString mNewFileIconStyle;
        QString mNewFileIconPath;
        QString mOpenProjIconPath;
        QString mOpenProjIconStyle;
        QString mCloseIconPath;
        QString mCloseIconStyle;
        QString mSaveIconPath;
        QString mSaveAsIconPath;

        QString mEnabledIconStyle;
        QString mDisabledIconStyle;
        SettingsItemKeybind* mSettingCreateFile;
        SettingsItemKeybind* mSettingOpenFile;
        SettingsItemKeybind* mSettingSaveFile;

    public Q_SLOTS:
        void handleFileStatusChanged(bool gateLibraryAffected, bool isDirty);
        void handleFileOpenChanged(bool gateLibraryAffected, bool isOpen);

    public:
        FileActions(QWidget* parent = nullptr);

        /**
         * Q_PROPERTY READ function for the 'New File'-icon path.
         *
         * @returns the 'New File'-icon path
         */
        QString newFileIconPath() const { return mNewFileIconPath; }

        /**
         * Q_PROPERTY READ function for the 'New File'-icon style.
         *
         * @returns the 'New File'-icon style
         */
        QString newFileIconStyle() const { return mNewFileIconStyle; }

        /**
         * Q_PROPERTY READ function for the 'Open Proj'-icon path.
         *
         * @returns the 'Open Proj'-icon path
         */
        QString openProjIconPath() const { return mOpenProjIconPath; }

        /**
         * Q_PROPERTY READ function for the 'Open Proj'-icon style.
         *
         * @returns the 'Open Proj'-icon style
         */
        QString openProjIconStyle() const { return mOpenProjIconStyle; }

        /**
         * Q_PROPERTY READ function for the 'Save File'-icon path.
         *
         * @returns the 'SaveFile'-icon path
         */
        QString saveIconPath() const {return mSaveIconPath; }

        /**
         * Q_PROPERTY READ function for the 'SaveAs File'-icon path.
         *
         * @returns the 'SaveAsFile'-icon path
         */
        QString saveAsIconPath() const { return mSaveAsIconPath; }

        /**
         * Q_PROPERTY READ function for the 'Close Project'-icon path.
         *
         * @returns the 'Close Project'-icon path
         */
        QString closeIconPath() const { return mCloseIconPath; }

        /**
         * Q_PROPERTY READ function for the 'Close Project'-icon style.
         *
         * @returns the 'Close Project'-icon style
         */
        QString closeIconStyle() const { return mCloseIconStyle; }

        /**
         * Q_PROPERTY READ function for the 'Enabled'-icon style.
         *
         * @returns the 'Enabled'-icon style
         */
        QString enabledIconStyle() const { return mEnabledIconStyle; }

        /**
         * Q_PROPERTY READ function for the 'Disabled'-icon style.
         *
         * @returns the 'Disabled'-icon style
         */
        QString disabledIconStyle() const { return mDisabledIconStyle; }

        /**
         * Q_PROPERTY WRITE function for the 'New File'-icon path.
         *
         * @param path - The new path
         */
        void setNewFileIconPath(const QString& path) { mNewFileIconPath = path; }

        /**
         * Q_PROPERTY WRITE function for the 'New File'-icon style.
         *
         * @param style - The new style
         */
        void setNewFileIconStyle(const QString &style) { mNewFileIconStyle = style; }

        /**
         * Q_PROPERTY WRITE function for the 'Open Proj'-icon path.
         *
         * @param path - The new path
         */
        void setOpenProjIconPath(const QString& path) { mOpenProjIconPath = path; }

        /**
         * Q_PROPERTY WRITE function for the 'Open Proj'-icon style.
         *
         * @param style - The new style
         */
        void setOpenProjIconStyle(const QString& style) { mOpenProjIconStyle = style; };

        /**
         * Q_PROPERTY WRITE function for the 'Save File'-icon path.
         *yle
         * @param path - The new path
         */
        void setSaveIconPath(const QString& path) { mSaveIconPath = path; }

        /**
         * Q_PROPERTY WRITE function for the 'SaveAs File'-icon path.
         *
         * @param path - The new path
         */
        void setSaveAsIconPath(const QString& path) { mSaveAsIconPath = path; }

        /**
         * Q_PROPERTY WRITE function for the 'Close Project'-icon path.
         *
         * @param path - The new path
         */
        void setCloseIconPath(const QString& path) { mCloseIconPath = path; }

        /**
         * Q_PROPERTY WRITE function for the 'Close Project'-icon style.
         *
         * @param style - The new style
         */

        void setCloseIconStyle(const QString& style) { mCloseIconStyle = style; }

        /**
         * Q_PROPERTY WRITE function for the 'Enabled'-icon style.
         *
         * @param style - The new style
         */
        void setEnabledIconStyle(const QString& style) { mEnabledIconStyle = style; }

        /**
         * Q_PROPERTY WRITE function for the 'Disabled'-icon style.
         *
         * @param style - The new style
         */
        void setDisabledIconStyle(const QString& style) { mDisabledIconStyle = style; }

        Action* create() const { return mActionCreate; }
        Action* open()   const { return mActionOpen; }
        Action* close()  const { return mActionClose; }
        Action* save()   const { return mActionSave; }
        Action* saveAs() const { return mActionSaveAs; }

        void setup(GateLibraryManager* glcw = nullptr, GuiPluginManager* plmgr = nullptr);

        void repolish();
    };
}
