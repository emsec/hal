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
    };
}
