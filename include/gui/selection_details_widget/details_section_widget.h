#pragma once

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace hal {
    class DetailsSectionWidget : public QWidget
    {
        Q_OBJECT
    public:
        DetailsSectionWidget(QWidget* content, const QString& txt, QWidget *parent = nullptr);
        DetailsSectionWidget(const QString& txt, QTableWidget* tab, QWidget *parent = nullptr);
        QTableWidget* table() const;
        void setRowCount(int rc);

        static void setDefaultTableStyle(QTableWidget* tab);

    private Q_SLOTS:
        void toggleBodyVisible();
        void handleGlobalSettingsChanged(void* sender, const QString& key, const QVariant& value);

    private:
        void hideEmpty();
        void bodyVisible();
        void constructor(const QString& txt);

        QVBoxLayout*  m_layout;
        QPushButton*  m_header;
        QWidget*      m_body;
        QTableWidget* m_table;
        QString       m_headerText;
        int           m_rows;
        bool          m_hideEmpty;
        bool          m_bodyVisible;
    };
}
