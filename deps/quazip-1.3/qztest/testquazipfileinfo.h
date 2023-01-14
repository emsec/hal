#ifndef TESTQUAZIPFILEINFO_H
#define TESTQUAZIPFILEINFO_H

#include <QtCore/QObject>

class TestQuaZipFileInfo : public QObject
{
    Q_OBJECT
public:
    explicit TestQuaZipFileInfo(QObject *parent = 0);
private slots:
    void getNTFSTime_data();
    void getNTFSTime();
    void getExtTime_data();
    void getExtTime();
    void getExtTime_issue43();
    void parseExtraField_data();
    void parseExtraField();
};

#endif // TESTQUAZIPFILEINFO_H
