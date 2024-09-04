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


namespace hal {
    class MainWindow;
    class GateLibraryManager;
    class SettingsItemKeybind;

    class FileActions : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveAsIconPath READ saveAsIconPath WRITE setSaveAsIconPath)
        Q_PROPERTY(QString enabledIconStyle READ enabledIconStyle WRITE setEnabledIconStyle)
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)

        MainWindow* mMainWindowReference;
        GateLibraryManager* mGatelibReference;

        Action* mActionSave;
        Action* mActionSaveAs;
        QString mSaveIconPath;
        QString mSaveAsIconPath;

        QString mEnabledIconStyle;
        QString mDisabledIconStyle;
        SettingsItemKeybind* mSettingSaveFile;

    private Q_SLOTS:

       /**
        * Q_SLOT to save the current project as a .hal file.
        * call to FileManager::emitSaveTriggered().
        */
        void handleSaveTriggered();

       /**
        * Q_SLOT to save the current project as a new .hal file.
        * Will query for new name.
        * call to FileManager::emitSaveTriggered().
        */
        void handleSaveAsTriggered();

    public Q_SLOTS:
        void handleFileStatusChanged(bool gateLibrary, bool isDirty);

    public:
        FileActions(QWidget* parent = nullptr);

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
     * Q_PROPERTY WRITE function for the 'Save File'-icon path.
     *
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

        Action* save() const { return mActionSave; }
        Action* saveAs() const { return mActionSaveAs; }

        void setup(GateLibraryManager* glcw = nullptr);

        void repolish();
    };
}
