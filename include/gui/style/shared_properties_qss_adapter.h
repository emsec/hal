//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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
    class shared_properties_qss_adapter : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString open_icon_path READ open_icon_path WRITE set_open_icon_path)
        Q_PROPERTY(QString open_icon_style READ open_icon_style WRITE set_open_icon_style)

        Q_PROPERTY(QString save_icon_path READ save_icon_path WRITE set_save_icon_path)
        Q_PROPERTY(QString save_icon_style READ save_icon_style WRITE set_save_icon_style)

        Q_PROPERTY(QString schedule_icon_path READ schedule_icon_path WRITE set_schedule_icon_path)
        Q_PROPERTY(QString schedule_icon_style READ schedule_icon_style WRITE set_schedule_icon_style)

        Q_PROPERTY(QString run_icon_path READ run_icon_path WRITE set_run_icon_path)
        Q_PROPERTY(QString run_icon_style READ run_icon_style WRITE set_run_icon_style)

        Q_PROPERTY(QString content_icon_path READ content_icon_path WRITE set_content_icon_path)
        Q_PROPERTY(QString content_icon_style READ content_icon_style WRITE set_content_icon_style)

        Q_PROPERTY(QString settings_icon_path READ settings_icon_path WRITE set_settings_icon_path)
        Q_PROPERTY(QString settings_icon_style READ settings_icon_style WRITE set_settings_icon_style)

    public:
        explicit shared_properties_qss_adapter(QWidget* parent = nullptr);
        static const shared_properties_qss_adapter* instance();

        void repolish();

        QString open_icon_path() const;
        QString open_icon_style() const;

        QString save_icon_path() const;
        QString save_icon_style() const;

        QString schedule_icon_path() const;
        QString schedule_icon_style() const;

        QString run_icon_path() const;
        QString run_icon_style() const;

        QString content_icon_path() const;
        QString content_icon_style() const;

        QString settings_icon_path() const;
        QString settings_icon_style() const;

        void set_open_icon_path(const QString& path);
        void set_open_icon_style(const QString& style);

        void set_save_icon_path(const QString& path);
        void set_save_icon_style(const QString& style);

        void set_schedule_icon_path(const QString& path);
        void set_schedule_icon_style(const QString& style);

        void set_run_icon_path(const QString& path);
        void set_run_icon_style(const QString& style);

        void set_content_icon_path(const QString& path);
        void set_content_icon_style(const QString& style);

        void set_settings_icon_path(const QString& path);
        void set_settings_icon_style(const QString& style);

        // LEFT PUBLIC INTENTIONALLY
        QString m_open_icon_path;
        QString m_open_icon_style;

        QString m_save_icon_path;
        QString m_save_icon_style;

        QString m_schedule_icon_path;
        QString m_schedule_icon_style;

        QString m_run_icon_path;
        QString m_run_icon_style;

        QString m_content_icon_path;
        QString m_content_icon_style;

        QString m_settings_icon_path;
        QString m_settings_icon_style;
    };
}
