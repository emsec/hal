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

#include "gui/expanding_list/expanding_list_button.h"
#include <QList>
#include <QMap>
#include <QWidget>

class QFrame;
class QHBoxLayout;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QScrollBar;

namespace hal
{
    class ExpandingListButton;
    class ExpandingListWidget;
    class Searchbar;
    class SettingsDisplay;
    class SettingsWidget;

    class MainSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit MainSettingsWidget(QWidget* parent = 0);
        bool handle_about_to_close();

    Q_SIGNALS:
        void close();

    public Q_SLOTS:
        void handle_restore_defaults_clicked();
        void handle_cancel_clicked();
        void handle_ok_clicked();
        void handle_button_selected(ExpandingListButton* button);
        void handle_text_edited(const QString& text);
        void handle_setting_updated(SettingsWidget* sender, const QString& key, const QVariant& value);

    private:
        void init_widgets();
        void make_section(const QString& label, const QString& name, const QString& icon_path);
        void register_widget(const QString& section_name, SettingsWidget* widget);
        bool check_conflict(SettingsWidget* widget, const QVariant& value) const;
        void make_exclusive_group(const QString& name);
        void assign_exclusive_group(const QString& group_name, SettingsWidget* widget);
        void release_exclusive_group(const QString& group_name, SettingsWidget* widget);
        void hide_all_settings();
        void show_all_settings();
        void remove_all_highlights();
        bool save_settings();
        void rollback_settings();

        QHBoxLayout* m_layout;
        ExpandingListWidget* m_expanding_list_widget;
        QVBoxLayout* m_vertical_layout;
        QScrollBar* m_scrollbar;

        QFrame* m_searchbar_container;
        QHBoxLayout* m_searchbar_layout;
        Searchbar* m_searchbar;

        QScrollArea* m_scroll_area;
        QFrame* m_content;
        QHBoxLayout* m_content_layout;
        QFrame* m_settings_container;
        QVBoxLayout* m_container_layout;

        QHBoxLayout* m_button_layout;

        QPushButton* m_restore_defaults;
        QPushButton* m_cancel;
        QPushButton* m_ok;

        QMap<QString, QList<SettingsWidget*>*> m_map;
        QMap<QString, ExpandingListButton*> m_sections;
        QString m_active_section;

        QList<SettingsWidget*> m_all_settings;

        QList<QString> m_exclusive_groups;
        QMap<SettingsWidget*, QString> m_exclusive_w2g;
        QMap<QString, QList<SettingsWidget*>*> m_exclusive_g2w;

        bool m_reset_to_first_element;
    };
}
