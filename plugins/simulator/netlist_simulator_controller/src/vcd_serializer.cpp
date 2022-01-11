#include "netlist_simulator_controller/vcd_serializer.h"
#include "netlist_simulator_controller/wave_data.h"
#include "hal_core/utilities/log.h"
#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <QDataStream>
#include <QFileInfo>
#include <QDir>

namespace hal {

const WaveData* mData;
QMap<int,int>::const_iterator mIterator;

    VcdSerializerElement::VcdSerializerElement(int inx, const WaveData* wd)
        : mIndex(inx), mData(wd)
    {
        mIterator = mData->data().constBegin();
    }

    void VcdSerializerElement::hasNextElement() const
    {

    }

    int VcdSerializerElement::time() const
    {
        if (mIterator == mData->data().constEnd()) return -1;
        return mIterator.key();
    }

    int VcdSerializerElement::value() const
    {
        return mIterator.value();
    }

    bool VcdSerializerElement::next()
    {
        return (++mIterator != mData->data().constEnd());
    }

    int VcdSerializerElement::priorityCode(int n) const
    {
        return n*time() + mIndex;
    }

    QString VcdSerializerElement::name() const
    {
        return mData->name();
    }

    QByteArray VcdSerializerElement::charCode() const
    {
        QByteArray retval;
        int z = mIndex;
        char firstChar = '!';
        do
        {
            retval += (char) (firstChar + z%92);
            z /= 92;
            if (z<92) firstChar = ' '; // most significant digit must be non-zero
        } while (z>0);
        return retval;
    }

//----------------------------------
    VcdSerializer::VcdSerializer(QObject *parent)
        : QObject(parent)
    {;}

    VcdSerializer::~VcdSerializer()
    {
        for (WaveData* wd : mWaves.values())
            delete wd;
    }

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
                QString line = QString("$var wire 1 %1 %2 $end\n").arg(QString::fromUtf8(vse->charCode())).arg(vse->name());
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

    bool VcdSerializer::parseDataNonDecimal(const QByteArray &line, int base)
    {
        QList<QByteArray> sl = line.split(' ');
        if (sl.size()!=2) return false;
        bool ok;
        int val = sl.at(0).toUInt(&ok,base);
        if (!ok)
        {
            if (mErrorCount++ < 5)
                log_warning("vcd_viewer", "Cannot parse VCD data value '{}'", std::string(sl.at(0).data()));
            val = 0;
        }
        storeValue(val, sl.at(1));
        // [return ok] return statement if we want to bail out upon data parse error
        return true; // ignore parse errors
    }

    bool VcdSerializer::parseDataline(const QByteArray &line)
    {
        if (line.isEmpty()) return true;
        switch(line.at(0))
        {
        case 'b': return parseDataNonDecimal(line.mid(1),2);
        case 'o': return parseDataNonDecimal(line.mid(1),8);
        case 'x': return parseDataNonDecimal(line.mid(1),16);
        }

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
            qDebug() << "cannot parse words starting with" << word.at(0) << line;
            return false;
        }
        return true;
    }

    void VcdSerializer::storeValue(int val, const QByteArray& abrev)
    {
        WaveData* wd = mWaves.value(abrev);
        if (!wd) return;
    //    Q_ASSERT(wd);
        wd->insert(mTime,val);
    }

    bool VcdSerializer::parseCsvHeader(char *buf)
    {
        int icol = 0;
        char* pos = buf;
        bool loop = (*pos != 0);
        QString abbrev;
        while (loop)
        {
            QByteArray header;
            while (*pos && *pos!=',' && *pos != '\n') header += *(pos++);
            loop = (*(pos++) == ',');
            if (!icol)
            {
                if (mSaleae) abbrev = QString::fromUtf8(header);
            }
            else
            {
                bool ok;
                if (!mSaleae) abbrev = QString::number(icol);
                u32 id = header.trimmed().toUInt(&ok);
                WaveData* wd = nullptr;
                if (ok)
                    wd = new WaveData(id, QString("net[%1]").arg(id));
                else
                {
                    QString name = QString::fromUtf8(header.trimmed());
                    int n = name.size() - 1;
                    if (n<2 || name.at(0) != '"' || name.at(n) != '"') return false;
                    wd = new WaveData(0,name.mid(1,n-1));
                }
                if (wd)
                {
                    mDictionary.insert(wd->name(),abbrev);
                    mWaves.insert(abbrev,wd);
                }
                else
                    return false;
            }
            icol++;
        }

        return true;
    }

    bool VcdSerializer::parseSalea(const QString& filenameStub, const QString& abbrev, u64 timeScale)
    {
        WaveData *wd = mWaves.value(abbrev);
        Q_ASSERT(wd);

        QString filename = filenameStub.arg(abbrev);
        QFile ff(filename);
        if (!ff.open(QIODevice::ReadOnly))
        {
            qDebug() << "TODO : hell";
            return false;
        }

        QDataStream bin(&ff);
        bin.setByteOrder(QDataStream::LittleEndian);

        char ident[9];
        qint32 version;
        qint32 type;
        quint32 value;
        double beginTime;
        double endTime;
        quint64 numTransitions;
        if (bin.readRawData(ident,8) < 8) return false;
        ident[8] = 0;
        bin >> version >> type;
        if (type != 0) return false; // read only digital data
        bin >> value >> beginTime >> endTime >> numTransitions;

//        qDebug() << ident << version << type << value << beginTime << endTime << numTransitions;

        double tDouble;

         wd->insert(0,value);

        for (quint64 i=0; i<numTransitions; i++)
        {
            bin >> tDouble;
            u64 tInt = (u64) floor ( (tDouble-beginTime) * timeScale + 0.5);
            value = value ? 0 : 1;
            wd->insert(tInt,value);
            if (tInt > mTime) mTime = tInt;
            if (bin.atEnd()) break;
        }

        return true;
    }

    bool VcdSerializer::parseCsvDataline(char* buf, int dataLineIndex, u64 timeScale)
    {
        int icol = 0;
        bool ok;
        char* pos = buf;
        bool loop = (*pos != 0);
        while (loop)
        {
            QByteArray value;
            while (*pos && *pos != ',' && *pos != '\n') value += *(pos++);
            loop = (*(pos++) == ',');
            if (!value.isEmpty())
            {
                if (icol)
                {
                    int ival = 0;
                    ok = true;
                    if (value.size()==1)
                        switch (value.at(0))
                        {
                        case '0': break;
                        case '1':
                            ival = 1;
                            break;
                        default:
                            ival = value.trimmed().toInt(&ok);
                            break;
                        }
                    else
                        ival = value.trimmed().toInt(&ok);
                    if (!ok) return false;

                    bool wdInsert = false;
                    if (icol >= mLastValue.size())
                    {
                        while (icol > mLastValue.size()) mLastValue.append(-99);
                        mLastValue.append(ival);
                        wdInsert = true;
                    }
                    else if (mLastValue.at(icol) != ival)
                    {
                        mLastValue[icol] = ival;
                        wdInsert = true;
                    }

                    if (wdInsert)
                    {
                        WaveData* wd = mWaves.value(QString::number(icol));
                        if (!wd) return false;
                        wd->insert(mTime,ival);
                    }
                }
                else
                {
                    // time
                    double tDouble = value.toDouble(&ok);
                    if (!ok) return false;
                    u64 tInt = (u64) floor ( tDouble * timeScale + 0.5);
                    if (!dataLineIndex)
                    {
                        mFirstTimestamp = tInt;
                        mTime = 0;
                    }
                    else
                        mTime = tInt - mFirstTimestamp;
                }
            }
            icol++;
        }
        return true;
    }

    bool VcdSerializer::deserializeCsv(const QString& filename, u64 timeScale)
    {
        mLastValue.clear();
        mErrorCount = 0;
        mWaves.clear();
        mTime = 0;
        mSaleae = false;

        static const int bufsize = 4095;
        char buf[bufsize+1];

        QFile ff(filename);
        if (!ff.open(QIODevice::ReadOnly))
        {
            log_warning("vcd_viewer", "Cannot open CSV input file '{}'.", filename.toStdString());
            return false;
        }

        bool parseHeader = true;
        int dataLineIndex = 0;
        while(!ff.atEnd())
        {
            int sizeRead = ff.readLine(buf,bufsize);
            if (sizeRead >= bufsize)
            {
                log_warning("vcd_viewer", "CSV line {} exceeds buffer size {}.", dataLineIndex, bufsize);
                return false;
            }

            if (sizeRead < 0)
            {
                log_warning("vcd_viewer", "CSV parse error reading line {} from file '{}'.", dataLineIndex, filename.toStdString());
                return false;
            }
            if (!sizeRead) continue;

            if (parseHeader)
            {
                if (sizeRead >= 8 && QByteArray(buf,8) == "<SALEAE>")
                {
                    mSaleae = true;
                    while (!ff.atEnd())
                    {
                        sizeRead = ff.readLine(buf,bufsize);
                        if (!parseCsvHeader(buf))
                        {
                           log_warning("vcd_viewer", "Cannot parse <SALEAEA> index line '{}'.", buf);
                           return false;
                        }
                    }
                    QFileInfo finfo(ff.fileName());
                    QString fstub = finfo.absoluteDir().absoluteFilePath("digital_%1.bin");
                    for (auto it= mWaves.begin(); it !=mWaves.end(); ++it)
                        if (!parseSalea(fstub,it.key(),timeScale))
                        {
                            log_warning("vcd_viewer", "Cannot parse <SALEAEA> binaray file '{}' '{}'.", fstub.toStdString(), it.key().toStdString());
                            return false;
                        }
                    return true;
                }

                if (!parseCsvHeader(buf))
                {
                    if (mErrorCount++ < 5)
                        log_warning("vcd_viewer", "Cannot parse CSV header line '{}'.", buf);
                    return false;
                }
                parseHeader = false;
            }
            else
            {
                if (!parseCsvDataline(buf,dataLineIndex++,timeScale))
                {
                    if (mErrorCount++ < 5)
                        log_warning("vcd_viewer", "Cannot parse CSV data line '{}'.", buf);
                    return false;
                }
            }
        }
        return true;
    }

    bool VcdSerializer::deserializeVcd(const QString& filename, const QStringList &netNames)
    {
        mErrorCount = 0;
        mWaves.clear();
        mTime = 0;
        QFile ff(filename);
        if (!ff.open(QIODevice::ReadOnly))
        {
            log_warning("vcd_viewer", "Cannot open VCD input file '{}'.", filename.toStdString());
            return false;
        }

        bool parseHeader = true;

        QRegularExpression reHead("\\$(\\w*) (.*)\\$end");
        QRegularExpression reWire("wire\\s+(\\d+) ([^ ]+) (.*) $");
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
                        bool ok;
                        QString wireName   = mWire.captured(3);
                        if (!netNames.isEmpty() && !netNames.contains(wireName)) continue;
                        QString wireAbbrev = mWire.captured(2);
                        int     wireBits   = mWire.captured(1).toUInt(&ok);
                        if (!ok) wireBits = 1;
                        mDictionary.insert(wireName,wireAbbrev);
                        QRegularExpressionMatch mWiid = reWiid.match(wireName);
                        if (!mWaves.contains(wireAbbrev))
                        {
                            WaveData* wd = new WaveData(mWiid.captured(1).toUInt(),wireName);
                            wd->setBits(wireBits);
                            mWaves.insert(mWire.captured(2),wd);
                        }
                    }
                }
            }
            else
            {
                if (!parseDataline(line))
                {
                    if (mErrorCount++ < 5)
                        log_warning("vcd_viewer", "Cannot parse VCD data line '{}'.", std::string(line.data()));
                    return false;
                }
            }
        }
        return true;
    }

    WaveData* VcdSerializer::waveByName(const QString& name)
    {
        auto it = mDictionary.find(name);
        if (it == mDictionary.constEnd()) return nullptr;
        auto jt = mWaves.find(it.value());
        Q_ASSERT(jt != mWaves.end());
        return new WaveData(*jt.value());
    }

    WaveData* VcdSerializer::waveById(u32 id)
    {
        if (!id) return nullptr;
        for (auto jt = mWaves.begin(); jt != mWaves.end(); ++jt)
        {
            if (jt.value()->id() == id)
            {
                return new WaveData(*jt.value());
                break;
            }
        }
        return nullptr;
    }
}
