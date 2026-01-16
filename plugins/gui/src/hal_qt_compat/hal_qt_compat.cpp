#include "gui/hal_qt_compat/hal_qt_compat.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QGuiApplication>
#include <QScreen>
#else
#include <QDesktopWidget>
#endif
namespace hal {

    QRect QtCompat::desktopGeometry()
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        return QGuiApplication::primaryScreen()->availableGeometry();
#else
        return qApp->desktop()->availableGeometry();
#endif
    }

    template<typename T>
    QSet<T> QtCompat::listToSet(const QList<T>& list)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return QSet<T>(list.constBegin(),list.constEnd());
#else
        return QSet<T>::fromList(list);
#endif
    }

    template<typename T>
    QList<T> setToList(const QSet<T>& cset)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return QList<T>(cset.constBegin(),cset.constEnd());
#else
        return QList<T>::fromset(list);
#endif
    }

    template<typename T>
    QVector<T> QtCompat::listToVector(const QList<T>& list)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return QVector<T>(list.constBegin(),list.constEnd());
#else
        return QVector<T>::fromList(list);
#endif
    }

    template<typename T> extern QVector<T> QtCompat::stdVectorToVector(const std::vector<T>& vec)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return QVector<T>(vec.begin(),vec.end());
#else
        return QVector<T>::fromStdVector(vec);
#endif
    }

    void QtCompat::setMarginWidth(QLayout* layout, int marginWidth)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        layout->setContentsMargins(marginWidth, marginWidth, marginWidth, marginWidth);
#else
        layout->setMargin(marginWith);
#endif
    }

    void QtCompat::styleOptionInit(QStyleOption& opt, QWidget* widget)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        opt.initFrom(widget);
#else
        opt.init(widget);
#endif
    }
}
