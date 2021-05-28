//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include <QWidget>

namespace hal
{
    /**
     * @ingroup unused
     */
    class SharedPropertiesQssAdapter : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString openIconPath READ openIconPath WRITE setOpenIconPath)
        Q_PROPERTY(QString openIconStyle READ openIconStyle WRITE setOpenIconStyle)

        Q_PROPERTY(QString saveIconPath READ saveIconPath WRITE setSaveIconPath)
        Q_PROPERTY(QString saveIconStyle READ saveIconStyle WRITE setSaveIconStyle)

        Q_PROPERTY(QString scheduleIconPath READ scheduleIconPath WRITE setScheduleIconPath)
        Q_PROPERTY(QString scheduleIconStyle READ scheduleIconStyle WRITE setScheduleIconStyle)

        Q_PROPERTY(QString runIconPath READ runIconPath WRITE setRunIconPath)
        Q_PROPERTY(QString runIconStyle READ runIconStyle WRITE setRunIconStyle)

        Q_PROPERTY(QString contentIconPath READ contentIconPath WRITE setContentIconPath)
        Q_PROPERTY(QString contentIconStyle READ contentIconStyle WRITE setContentIconStyle)

        Q_PROPERTY(QString settingsIconPath READ settingsIconPath WRITE setSettingsIconPath)
        Q_PROPERTY(QString settingsIconStyle READ settingsIconStyle WRITE setSettingsIconStyle)

    public:
        explicit SharedPropertiesQssAdapter(QWidget* parent = nullptr);
        static const SharedPropertiesQssAdapter* instance();

        void repolish();

        QString openIconPath() const;
        QString openIconStyle() const;

        QString saveIconPath() const;
        QString saveIconStyle() const;

        QString scheduleIconPath() const;
        QString scheduleIconStyle() const;

        QString runIconPath() const;
        QString runIconStyle() const;

        QString contentIconPath() const;
        QString contentIconStyle() const;

        QString settingsIconPath() const;
        QString settingsIconStyle() const;

        void setOpenIconPath(const QString& path);
        void setOpenIconStyle(const QString& style);

        void setSaveIconPath(const QString& path);
        void setSaveIconStyle(const QString& style);

        void setScheduleIconPath(const QString& path);
        void setScheduleIconStyle(const QString& style);

        void setRunIconPath(const QString& path);
        void setRunIconStyle(const QString& style);

        void setContentIconPath(const QString& path);
        void setContentIconStyle(const QString& style);

        void setSettingsIconPath(const QString& path);
        void setSettingsIconStyle(const QString& style);

        // LEFT PUBLIC INTENTIONALLY
        QString mOpenIconPath;
        QString mOpenIconStyle;

        QString mSaveIconPath;
        QString mSaveIconStyle;

        QString mScheduleIconPath;
        QString mScheduleIconStyle;

        QString mRunIconPath;
        QString mRunIconStyle;

        QString mContentIconPath;
        QString mContentIconStyle;

        QString mSettingsIconPath;
        QString mSettingsIconStyle;
    };
}
