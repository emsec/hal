#pragma once

#include <QWidget>
#include <QLabel>

namespace hal
{
    class DisputedBigIcon : public QLabel
    {
        Q_OBJECT
    public:
        DisputedBigIcon(const QString& iconName, QWidget* parent = nullptr);
    public Q_SLOTS:
        void handleGlobalSettingsChanged(void* sender, const QString& key, const QVariant& value);
    private:
        void showOrHide();
        bool mIsVisible;
    };
}
