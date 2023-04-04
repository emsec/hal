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

#include <QDialog>
#include "hal_core/plugin_system/plugin_parameter.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include <QMap>
#include <QList>

class QFormLayout;
class QDialogButtonBox;
class QPushButton;
class QLineEdit;

namespace hal {

    class BasePluginInterface;

    class PluginParameterFileDialog : public QWidget
    {
        Q_OBJECT

        PluginParameter mParameter;
        QPushButton* mButton;
        QLineEdit* mEditor;
    private Q_SLOTS:
        void handleActivateFileDialog();

    public:
        PluginParameterFileDialog(const PluginParameter& par, QWidget* parent = nullptr);
        QString getFilename() const;
    };

    class PluginParameterDialog : public QDialog
    {
        Q_OBJECT

        QString mPluginName;
        GuiExtensionInterface* mGuiExtensionInterface;
        QList<PluginParameter> mParameterList;
        QMap<QString,QWidget*> mWidgetMap;

        QMap<QString,QString> mTabNames;

    private:
        void setupHash();
        void setupForm(QFormLayout* form, const QString& tabTag = QString());
        QDialogButtonBox* setupButtonBox();

    private Q_SLOTS:
        void handlePushbuttonClicked();

    public Q_SLOTS:
        void accept() override;

    public:
        PluginParameterDialog(const QString& pname, GuiExtensionInterface* geif, QWidget* parent = nullptr);
    };
}
