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

#include <QSettings>
#include <QStringList>
#include <QMap>

namespace hal {
    class SimulationSettings : public QSettings
    {
        Q_OBJECT
    public:
        enum ColorSetting { WaveformRegular, WaveformSelected, WaveformUndefined, ValueX, Value0, Value1, MaxColorSetting };
    private:
        static const char* sColorSettingTag[MaxColorSetting];
        static const char* sDefaultColor[MaxColorSetting];
    public:
        SimulationSettings(const QString& filename);
        QString color(ColorSetting cs) const;
        void setColor(ColorSetting cs, const QString& colName);

        QMap<QString,QString> engineProperties() const;
        void setEngineProperties(const QMap<QString,QString>& engProp);

        int maxSizeLoadable() const;
        void setMaxSizeLoadable(int msl);

        int maxSizeEditor() const;
        void setMaxSizeEditor(int mse);

        QString baseDirectory() const;
        void setBaseDirectory(const QString& dir);
    };
}
