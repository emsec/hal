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

#include "settings/preview_widget.h"

#include <QVariant>
#include <QToolButton>
#include <QBoxLayout>
#include <QFrame>
#include <QPair>

class QColor;
class QLabel;
class QRegularExpression;
class QVBoxLayout;

namespace hal
{
    /**
     * Widget that is bound to a global setting and allows the user to edit that
     * setting's value.<br>
     * This is an abstract class, see the implementations for the different data
     * types.
     */
    class SettingsWidget : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QColor highlight_color READ highlight_color WRITE set_highlight_color)
        Q_PROPERTY(bool dirty READ dirty WRITE set_dirty)
        Q_PROPERTY(bool conflicts READ conflicts WRITE set_conflicts)

    public:
        /**
         * Specifies the position of the preview widget.
         */
        enum class preview_position
        {
            bottom = 0,
            right = 1
        };

        /**
         * Constructs a new settings widget for the specified setting.
         * 
         * @param[in] key - The key of the setting to bind to.
         * @param[in] parent - The Qt parent object.
         */
        explicit SettingsWidget(const QString& key, QWidget* parent = 0);

        /**
         * Get the search result's highlight color.
         * 
         * @returns The highlight color.
         */
        QColor highlight_color();
        
        /**
         * Get the key of the setting this widget is bound to.
         * 
         * @return The setting's key.
         */
        QString key();

        /**
         * Set the search result's highlight color.
         * 
         * @param[in] color - The new highlight color.
         */
        void set_highlight_color(const QColor& color);

        /**
         * Reset the search highlight.
         */
        void reset_labels();

        /**
         * Highlight the search phrase on the labels.
         * 
         * @param[in] string - The search phrase.
         * @return True iff the search phrase matched at least once.
         */
        bool match_labels(const QString& string);

        /**
         * Check whether the widget has unsaved changes.
         * 
         * @return True iff the widget has unsaved changes.
         */
        bool dirty() const;

        /**
         * Make the widget ready for display by loading the specified
         * values and marking the widget not dirty.
         * 
         * @param[in] value - The value to load.
         * @param[in] default_value - The default value to load when the user clicks "default".
         */
        void prepare(const QVariant& value, const QVariant& default_value);
        
        /**
         * Mark the widget not dirty and accept the current value.<br>
         * If the user makes another change and clicks "undo", the current
         * value will now be restored instead of the one the widget was
         * initialized with.
         */
        void mark_saved();

        /**
         * Update the conflict flag, showing whether the widget's value conflicts
         * with another setting, such as a keybind assigned to more than one
         * function.
         * Setting the flag will show the widget in a warning state.
         * 
         * @param[in] conflicts - The conflict flag.
         */
        void set_conflicts(bool conflicts);

        /**
         * Get the conflict flag.
         * 
         * @returns The conflict flag.
         */
        bool conflicts() const;

        /**
         * Insert or replace the preview widget and update it.
         * 
         * @param[in] widget - The preview widget.
         */
        void set_preview_widget(PreviewWidget* widget);

        /**
         * Set the preview widget's position.<br>
         * This can be safely called before or after the preview
         * widget itself is set.
         * 
         * @param[in] position - The position value.
         */
        void set_preview_position(preview_position position);

        /**
         * Initialize the widget's display with the given value.<br>
         * Implementations for different data types are required to override
         * this method.
         * 
         * @param[in] value - The value to load.
         */
        virtual void load(const QVariant& value)  = 0;

        /**
         * Get the value the widget is currently displaying.<br>
         * Implementations for different data types are required to override
         * this method.
         * 
         * @returns The current value.
         */
        virtual QVariant value()                  = 0;

    public Q_SLOTS:
        /**
         * Discard all unsaved user changes.
         */
        void handle_rollback();

        /**
         * Revert to the default value.
         */
        void handle_reset();

    Q_SIGNALS:
        /**
         * Signals that the user has changed the value of this setting.
         * The @see MainSettingsWidget subscribes to this signal.
         * 
         * @param[in] sender - The signal's sender.
         * @param[in] key - The setting's key.
         * @param[in] val - The setting's new value.
         */
        void setting_updated(SettingsWidget* sender, const QString& key, const QVariant& value);

    protected:
        void set_dirty(bool dirty);
        void trigger_setting_updated();

        QVBoxLayout* m_layout;
        QBoxLayout* m_container;
        QHBoxLayout* m_top_bar;
        QLabel* m_name;
        QToolButton* m_revert;
        QToolButton* m_default;

        QList<QPair<QLabel*, QString>> m_labels;

    private:
        QColor m_highlight_color;
        QString m_key;
        bool m_signals_enabled = true;
        bool m_prepared = false;
        bool m_dirty = false;
        bool m_conflicts = false;
        QVariant m_loaded_value;
        QVariant m_default_value;
        PreviewWidget* m_preview = nullptr;
    };
}
