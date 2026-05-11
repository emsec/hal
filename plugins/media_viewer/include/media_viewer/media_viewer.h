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

#include "hal_core/defines.h"
#include "gui/content_manager/content_manager.h"

#include <QMediaPlayer>

class QAction;
class QAudioOutput;
class QLabel;
class QStackedWidget;
class QVideoWidget;
class QSlider;
class QComboBox;
class QToolButton;

namespace hal
{
    class Toolbar;

    /**
     * @class MediaViewer
     * @brief Local video player widget integrated as a HAL external content widget.
     */
    class NETLIST_API MediaViewer : public ExternalContentWidget
    {
        Q_OBJECT
    public:
        /**
         * @brief Constructor.
         * @param pluginName The plugin name, forwarded to ExternalContentWidget.
         * @param parent Optional parent QObject.
         */
        MediaViewer(const QString& pluginName, QObject* parent = nullptr);

        /** @brief Destructor. */
        ~MediaViewer();

        /** @brief Populate the toolbar — Open button only; transport lives in the in-widget control bar. */
        void setupToolbar(Toolbar* toolbar) override;

        /** @brief Return the currently open MediaViewer instance, or nullptr if none. */
        static MediaViewer* getInstance();

        /**
         * @brief Load a media file from the given path and pause at position 0.
         * @param fileName Absolute path to the media file.
         */
        void loadMediaFile(const QString& fileName);

    private Q_SLOTS:
        /** @brief Open the OS file dialog and load the selected media file. */
        void handleOpenInputFileDialog();

        /** @brief Show the keyboard shortcut tooltip at the cursor position. */
        void handleHelpTriggered();

        /** @brief Toggle between play and pause. */
        void handlePlayPauseToggle();

        /** @brief React to QMediaPlayer playback-state changes to update the play/pause button. */
        void handlePlaybackStateChanged(QMediaPlayer::PlaybackState state);

        /** @brief Trigger first-frame decode once the media is loaded. */
        void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);

        /** @brief React to position changes to drive the seek slider and time label. */
        void handlePositionChanged(qint64 position);

        /** @brief React to duration changes to configure the seek slider range and time format. */
        void handleDurationChanged(qint64 duration);

        /** @brief Seek to the position chosen by the user dragging the seek slider. */
        void handleSeekSliderMoved(int value);

        /** @brief Apply the speed selected in the combo box. */
        void handleSpeedChanged(int index);

        /** @brief Apply the volume set by the slider. */
        void handleVolumeChanged(int value);

        /** @brief Toggle mute on the audio output. */
        void handleMuteToggle();

        /** @brief Handle QMediaPlayer error signals. */
        void handleMediaError(QMediaPlayer::Error err, const QString& msg);

    private:
        /** @brief Return the tooltip text listing all keyboard shortcuts. */
        static QString shortcutHelpText();

        /** @brief Clamp the current volume by delta into [0,100] and update slider + audio output. */
        void adjustVolume(int delta);

        /** @brief Seek by deltaMs milliseconds, clamped to [0, duration]. */
        void seekRelative(qint64 deltaMs);

        /** @brief Format a millisecond value using the currently chosen time format string. */
        QString formatTime(qint64 ms) const;

        /** @brief Update the time label from current position and duration. */
        void updateTimeLabel();


        enum DisplayPage { PageEmpty = 0, PageVideo = 1, PageError = 2 };

        // Player pipeline
        QMediaPlayer*    mPlayer        = nullptr;
        QAudioOutput*    mAudioOutput   = nullptr;
        QVideoWidget*    mVideoWidget   = nullptr;

        // Display area: stacked widget with empty-state, video, and error pages
        QStackedWidget*  mDisplayStack  = nullptr;
        QLabel*          mErrorLabel    = nullptr;

        // Toolbar
        QAction*       mOpenAction    = nullptr;
        QAction*       mHelpAction    = nullptr;

        // In-widget control bar
        QToolButton*   mPlayPauseButton = nullptr;
        QSlider*       mSeekSlider      = nullptr;
        QLabel*        mTimeLabel       = nullptr;
        QComboBox*     mSpeedCombo      = nullptr;
        QToolButton*   mMuteButton      = nullptr;

        // Horizontal volume slider, always visible to the right of the mute button
        QSlider*       mVolumeSlider    = nullptr;

        QString        mTimeFormat;           // "mm:ss" or "hh:mm:ss", chosen on durationChanged
        bool           mUpdatingSlider    = false;
        bool           mMuted             = false;
        bool           mSeekingFirstFrame = false;  // true while auto-playing to decode frame 0
    };
}    // namespace hal
