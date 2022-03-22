#include "netlist_simulator_controller/simulation_settings.h"

namespace hal {

    const char* SimulationSettings::sColorSettingTag[MaxColorSetting] = {"wv_regular", "wv_selected", "wv_undefined", "value_x", "value_0", "value_1" };
    const char* SimulationSettings::sDefaultColor[MaxColorSetting] = {"#10E0FF", "#F0F8FF", "#C08010", "#707071", "#102080", "#802010" };

    SimulationSettings::SimulationSettings(const QString &filename)
        : QSettings(filename,QSettings::IniFormat)
    {;}

    QString SimulationSettings::color(ColorSetting cs) const
    {
        QString tagname = QString("color/%1").arg(sColorSettingTag[cs]);
        QString defColor(sDefaultColor[cs]);
        return value(tagname,defColor).toString();
    }

    void SimulationSettings::setColor(ColorSetting cs, const QString& colName)
    {
        QString tagname = QString("color/%1").arg(sColorSettingTag[cs]);
        setValue(tagname,colName);
    }

    QMap<QString,QString> SimulationSettings::engineProperties() const
    {
        QStringList engProp = value("engine/properties").toStringList();
        QMap<QString,QString> retval;
        int n = engProp.size();
        for (int i=0; i<n; i+=2)
            retval[engProp.at(i)] = (i+1<n) ? engProp.at(i+1) : QString();
        return retval;
    }

    void SimulationSettings::setEngineProperties(const QMap<QString,QString>& engProp)
    {
        QStringList slist;
        for (auto it = engProp.constBegin(); it != engProp.constEnd(); ++it)
            slist << it.key() << it.value();
        setValue("engine/properties",slist);
    }

    int SimulationSettings::maxSizeLoadable() const
    {
        return value("global/max_loadable",100000).toInt();
    }

    void SimulationSettings::setMaxSizeLoadable(int msl)
    {
        setValue("global/max_loadable",msl);
    }
}
