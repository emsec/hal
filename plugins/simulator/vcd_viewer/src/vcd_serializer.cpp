#include "vcd_viewer/vcd_serializer.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

namespace hal {

const WaveData* mData;
QMap<int,int>::const_iterator mIterator;

    VcdSerializerElement::VcdSerializerElement(int inx, const WaveData* wd)
        : mIndex(inx), mData(wd)
    {
        mIterator = mData->constBegin();
    }

    void VcdSerializerElement::hasNextElement() const
    {

    }

    int VcdSerializerElement::time() const
    {
        if (mIterator == mData->constEnd()) return -1;
        return mIterator.key();
    }

    int VcdSerializerElement::value() const
    {
        return mIterator.value();
    }

    bool VcdSerializerElement::next()
    {
        return (++mIterator != mData->constEnd());
    }

    int VcdSerializerElement::priorityCode(int n) const
    {
        return n*time() + mIndex;
    }

    QString VcdSerializerElement::name() const
    {
        return mData->name();
    }

    char VcdSerializerElement::charCode() const
    {
        return (char) ('!' + mIndex);
    }

//----------------------------------
    VcdSerializer::VcdSerializer(QObject *parent)
        : QObject(parent)
    {;}

    bool VcdSerializer::serialize(const QString& filename, const QList<const WaveData*>& waves) const
    {
        QFile of(filename);
        if (!of.open(QIODevice::WriteOnly)) return false;

        of.write(QByteArray("$scope module top_module $end\n"));

        int dumpTime = -1;
        QMap<int,VcdSerializerElement*> priorityMap;
        int n = waves.size();

        for (int i=0; i<n; i++)
        {
            VcdSerializerElement* vse = new VcdSerializerElement(i,waves.at(i));
            int prio = vse->priorityCode(n);
            if (prio<0)
                delete vse;
            else
            {
                QString line = QString("$var wire 1 %1 %2 $end\n").arg(vse->charCode()).arg(vse->name());
                of.write(line.toUtf8());
                priorityMap.insert(prio,vse);
            }
        }

        of.write(QByteArray("$upscope $end\n$enddefinitions $end\n"));

        while (!priorityMap.isEmpty())
        {
            auto it = priorityMap.begin();
            VcdSerializerElement* vse = it.value();
            if (vse->time() != dumpTime)
            {
                of.write('#' + QByteArray::number(vse->time()) + '\n');
                dumpTime = vse->time();
            }
            of.write(QByteArray::number(vse->value()) + vse->charCode() + '\n');
            if (vse->next())
                priorityMap.insert(vse->priorityCode(n),vse);
            priorityMap.erase(it);
        }
        return true;
    }

    void VcdSerializer::parseDataline(const QByteArray &line)
    {
        for (QByteArray word : line.split(' '))
        {
            bool ok;
            if (word.isEmpty()) continue;
            switch (word.at(0))
            {
            case '#':
                mTime = word.mid(1).toUInt(&ok);
                Q_ASSERT(ok);
                continue;
            case 'x':
                storeValue(-1, word.mid(1));
                continue;
            case 'z':
                storeValue(-2, word.mid(1));
                continue;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                storeValue(word.at(0)-'0', word.mid(1));
                continue;
            }
            qDebug() << "cannot parse words starting with" << word.at(0);
        }
    }

    void VcdSerializer::storeValue(int val, const QByteArray& abrev)
    {
        WaveData* wd = mWaves.value(abrev);
        Q_ASSERT(wd);
        wd->insert(mTime,val);
    }

    QList<WaveData*> VcdSerializer::deserialize(const QString& filename)
    {
        mWaves.clear();
        mTime = 0;
        QFile ff(filename);
        if (!ff.open(QIODevice::ReadOnly)) return QList<WaveData*>();

        bool parseHeader = true;

        QRegularExpression reHead("\\$(\\w*) (.*)\\$end");
        QRegularExpression reWire("wire (\\d+) ([^ ]+) (.*) $");
        QRegularExpression reWiid("\\[(\\d+)\\]$");
        for (QByteArray line : ff.readAll().split('\n'))
        {
            if (parseHeader)
            {
                QRegularExpressionMatch mHead = reHead.match(line);
                if (mHead.hasMatch())
                {
                    if (mHead.captured(1) == "enddefinitions")
                        parseHeader = false;
                    else if (mHead.captured(1) == "var")
                    {
                        QRegularExpressionMatch mWire = reWire.match(mHead.captured(2));
                        QString wireName = mWire.captured(3);
                        QRegularExpressionMatch mWiid = reWiid.match(wireName);
                        mWaves.insert(mWire.captured(2),new WaveData(mWiid.captured(1).toUInt(),wireName));
                    }
                }
            }
            else
            {
                parseDataline(line);
           }
        }
        return mWaves.values();
    }

}
