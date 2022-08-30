#include "netlist_simulator_controller/vcd_serializer.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/saleae_writer.h"
#include "netlist_simulator_controller/saleae_parser.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/wave_data.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/net.h"
#include <QRegularExpression>
#include <QDebug>
#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <math.h>
#include <QCoreApplication>

namespace hal {

    const int maxErrorMessages = 3;

    VcdSerializerElement::VcdSerializerElement(int inx, const WaveData* wd)
        : mIndex(inx), mData(wd), mTime(0), mValue(SaleaeDataTuple::sReadError)
    {;}

    QString VcdSerializerElement::name() const
    {
        return mData->name();
    }

    bool VcdSerializerElement::hasData() const
    {
        return mValue != SaleaeDataTuple::sReadError;
    }

    void VcdSerializerElement::reset()
    {
        mValue = SaleaeDataTuple::sReadError;
        mTime = 0;
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
    VcdSerializer::VcdSerializer(const QString& workdir, bool saleae_cli, QObject *parent)
        : QObject(parent), mSaleaeWriter(nullptr), mWorkdir(workdir), mLastProgress(-1)
    {
        if (!mWorkdir.isEmpty() && !saleae_cli)
        {
            QDir saleaeDir = QDir(mWorkdir).absoluteFilePath("saleae");
            mSaleaeDirectoryFilename = saleaeDir.absoluteFilePath("saleae.json");
        }
        else
        {
            mSaleaeDirectoryFilename = workdir + "/saleae.json";
        }
    }


    void VcdSerializer::deleteFiles()
    {
        mSaleaeFiles.clear();
        mAbbrevByName.clear();
        memset(mErrorCount, 0, sizeof(mErrorCount));
    }

    void VcdSerializer::writeVcdEvent(QFile&of)
    {
        if (mTime < mFirstTimestamp || mTime > mLastTimestamp) return;
        bool first = true;
        for (VcdSerializerElement* vse : mWriteElements)
        {
            if (vse->hasData())
            {
                if (first)
                {
                    of.write('#' + QByteArray::number((qulonglong)vse->time()) + '\n');
                    first = false;
                }
                of.write(QByteArray::number(vse->value()) + vse->charCode() + '\n');
                vse->value();
                vse->reset();
            }
        }
    }

    bool VcdSerializer::exportVcd(const QString &filename, const QList<const WaveData*>& waves, u32 startTime, u32 endTime)
    {
        //int test_first_t = 45000;
        //int test_last_t = 100000;

        mFirstTimestamp = startTime;
        mLastTimestamp  = endTime;
        if (waves.isEmpty()) return false;
        SaleaeParser parser(mSaleaeDirectoryFilename.toStdString());
        QFile of(filename);
        if (!of.open(QIODevice::WriteOnly)) return false;

        mTime = 0;
        of.write(QByteArray("$scope module top_module $end\n"));

        int n = waves.size();

        for (int i=0; i<n; i++)
        {
            const WaveData* wd = waves.at(i);
            VcdSerializerElement* vse = new VcdSerializerElement(i,wd);
            mWriteElements.append(vse);
            parser.register_callback(wd->name().toStdString(),wd->id(),[this,&of](const void* obj, uint64_t t, int val) {
                if (t != mTime)
                {
                    writeVcdEvent(of);
                    mTime = t;
                }
                VcdSerializerElement* vse = (VcdSerializerElement*) obj;
                vse->setEvent(t,val);
            },vse);
            QString line = QString("$var wire 1 %1 %2 $end\n").arg(QString::fromUtf8(vse->charCode())).arg(vse->name());
            of.write(line.toUtf8());
        }

        of.write(QByteArray("$upscope $end\n$enddefinitions $end\n"));

        while (parser.next_event()) {;}
        for (VcdSerializerElement* vse : mWriteElements)
            delete vse;
        mWriteElements.clear();

        return true;
    }

    bool VcdSerializer::parseVcdDataNonDecimal(const QByteArray &line, int base)
    {
        QList<QByteArray> sl = line.split(' ');
        if (sl.size()!=2) return false;
        bool ok;
        int val = sl.at(0).toUInt(&ok,base);
        if (!ok)
        {
            if (mErrorCount[0]++ < maxErrorMessages)
                log_warning("waveform_viewer", "Cannot parse VCD data value '{}'", std::string(sl.at(0).data()));
            val = 0;
        }
        storeValue(val, sl.at(1));
        // [return ok] return statement if we want to bail out upon data parse error
        return true; // ignore parse errors
    }

    bool VcdSerializer::parseVcdDataline(char *buf, int len)
    {
        int pos = 0;
        while (len)
        {
            int val = -1;
            switch(*(buf+pos))
            {
            case 'b': return true; // parseVcdDataNonDecimal(QByteArray(buf+pos+1,len-1),2);
            case 'o': return true; //parseVcdDataNonDecimal(QByteArray(buf+pos+1,len-1),8);
            case 'h': return true; // parseVcdDataNonDecimal(QByteArray(buf+pos+1,len-1),16);
            case '$':
            {
                QByteArray testKeyword = QByteArray(buf+pos+1,len-1);
                if (testKeyword.startsWith("dumpvars") || testKeyword.startsWith("end"))
                    return true;
                return false;
            }
            case '#':
            {
                bool ok;
                mTime = QByteArray(buf+pos+1,len-1).toULongLong(&ok);
                Q_ASSERT(ok);
                return true;
            }
            case 'x':
                val = -1;
                break;
            case 'z':
                val = -2;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                val = *(buf+pos)-'0';
                break;
            default:
                qDebug() << "cannot parse dataline entries starting with" << *(buf+pos) << buf;
                return false;
            }
            int p = pos+1;
            while (p<len && buf[p]>' ') ++p;
            Q_ASSERT(p > pos+1);
            int abbrevLen = p - pos - 1;
            storeValue(val,QByteArray(buf+pos+1,abbrevLen));
            pos = p;
            len -= (abbrevLen+1) ;
            while (buf[pos]==' ' && len > 0)
            {
                pos++;
                len--;
            }
        }
        return true;
    }

    void VcdSerializer::storeValue(int val, const QByteArray& abrev)
    {
        SaleaeOutputFile* sof = mSaleaeFiles.value(abrev);
        if (!sof) return;
    //    Q_ASSERT(wd);
        sof->writeTimeValue(mTime,val);
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
                QString name;
                u32 id = header.trimmed().toUInt(&ok);
                if (ok && id)
                    name = QString("net[%1]").arg(id);
                else
                {
                    name = QString::fromUtf8(header.trimmed());
                    int n = name.size() - 1;
                    if (n<2 || name.at(0) != '"' || name.at(n) != '"') return false;
                    name = name.mid(1,n-1);
                    id = 0;
                }
                if (!name.isEmpty() || id)
                {
                    SaleaeOutputFile* sof = mSaleaeWriter->add_or_replace_waveform(name.toStdString(),0);
                    if (!sof) return false;
                    mSaleaeFiles.insert(abbrev,sof);
                }
                else
                    return false;
            }
            icol++;
        }

        return true;
    }


    bool VcdSerializer::parseCsvDataline(char* buf, int dataLineIndex)
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
                        SaleaeOutputFile* sof = mSaleaeFiles.value(QString::number(icol));
                        if (!sof) return false;
                        sof->writeTimeValue(mTime,ival);
                    }
                }
                else
                {
                    // time
                    double tDouble = value.toDouble(&ok);
                    if (!ok) return false;
                    u64 tInt = (u64) floor ( tDouble * SaleaeParser::sTimeScaleFactor + 0.5);
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

    bool VcdSerializer::importCsv(const QString& csvFilename, const QString& workdir, const QList<const Net*>& onlyNets, u64 timeScale)
    {
        mWorkdir = workdir.isEmpty() ? QDir::currentPath() : workdir;
        mLastValue.clear();
        deleteFiles();
        mTime = 0;
        mSaleae = false;

        SaleaeParser::sTimeScaleFactor = timeScale;

        QFile ff(csvFilename);
        if (!ff.open(QIODevice::ReadOnly))
        {
            log_warning("waveform_viewer", "Cannot open CSV input file '{}'.", csvFilename.toStdString());
            return false;
        }

        createSaleaeDirectory();
        mSaleaeWriter =  new SaleaeWriter(mSaleaeDirectoryFilename.toStdString());

        bool retval = parseCsvInternal(ff, onlyNets);

        delete mSaleaeWriter;
        mSaleaeWriter = nullptr;
        mSaleaeFiles.clear();

        if (retval) emitImportDone();
        return retval;
    }

    void VcdSerializer::emitProgress(double step, double max)
    {
        NetlistSimulatorController* nsc = static_cast<NetlistSimulatorController*>(parent());
        if (!nsc) return;
        int percent = floor(step*100 / max + 0.5);
        if (percent == mLastProgress) return;
        nsc->emitLoadProgress(percent);
        mLastProgress = percent;
        qApp->processEvents();
    }

    void VcdSerializer::emitImportDone()
    {
        NetlistSimulatorController* nsc = static_cast<NetlistSimulatorController*>(parent());
        if (!nsc) return;
        nsc->emitLoadProgress(-1);
        mLastProgress = -1;
    }

    void VcdSerializer::createSaleaeDirectory()
    {
        QDir saleaeDir(QDir(mWorkdir).absoluteFilePath("saleae"));
        saleaeDir.mkpath(saleaeDir.absolutePath());
        mSaleaeDirectoryFilename = saleaeDir.absoluteFilePath("saleae.json");
    }

    bool VcdSerializer::parseCsvInternal(QFile& ff, const QList<const Net *>& onlyNets)
    {
        QMap<QString, const Net*> netNames;
        for (const Net* n : onlyNets)
            netNames.insert(QString::fromStdString(n->get_name()),n);

        static const int bufsize = 4095;
        char buf[bufsize+1];

        bool parseHeader = true;
        int dataLineIndex = 0;
        while(!ff.atEnd())
        {
            int sizeRead = ff.readLine(buf,bufsize);
            if (sizeRead >= bufsize)
            {
                if (mErrorCount[1]++ < maxErrorMessages)
                    log_warning("waveform_viewer", "CSV line {} exceeds buffer size {}.", dataLineIndex, bufsize);
                return false;
            }

            if (sizeRead < 0)
            {
                if (mErrorCount[2]++ < maxErrorMessages)
                    log_warning("waveform_viewer", "CSV parse error reading line {} from file '{}'.", dataLineIndex, ff.fileName().toStdString());
                return false;
            }
            if (!sizeRead) continue;

            if (parseHeader)
            {
                if (!parseCsvHeader(buf))
                {
                    if (mErrorCount[3]++ < maxErrorMessages)
                        log_warning("waveform_viewer", "Cannot parse CSV header line '{}'.", buf);
                    return false;
                }
                parseHeader = false;
            }
            else
            {
                if (!parseCsvDataline(buf,dataLineIndex++))
                {
                    if (mErrorCount[4]++ < maxErrorMessages)
                        log_warning("waveform_viewer", "Cannot parse CSV data line '{}'.", buf);
                    return false;
                }
            }
        }

        return true;
    }

    bool VcdSerializer::importVcd(const QString& vcdFilename, const QString& workdir, const QList<const Net*>& onlyNets)
    {
        mWorkdir = workdir.isEmpty() ? QDir::currentPath() : workdir;
        deleteFiles();
        mTime = 0;
        QFile ff(vcdFilename);
        if (!ff.open(QIODevice::ReadOnly))
        {
            log_warning("waveform_viewer", "Cannot open VCD input file '{}'.", vcdFilename.toStdString());
            return false;
        }

        createSaleaeDirectory();
        mSaleaeWriter =  new SaleaeWriter(mSaleaeDirectoryFilename.toStdString());

        bool retval = parseVcdInternal(ff,onlyNets);

        delete mSaleaeWriter;
        mSaleaeWriter = nullptr;
        mSaleaeFiles.clear();
        mAbbrevByName.clear();

        if (retval) emitImportDone();
        return retval;
    }

    bool VcdSerializer::parseVcdInternal(QFile& ff, const QList<const Net*>& onlyNets)
    {
        bool parseHeader = true;

        QMap<QString, const Net*> netNames;
        for (const Net* n : onlyNets)
            netNames.insert(QString::fromStdString(n->get_name()),n);

        QRegularExpression reHead("\\$(\\w*) (.*)\\$end");
        QRegularExpression reWire("wire\\s+(\\d+) ([^ ]+) (.*) $");

        quint64 fileSize = ff.size();
        quint64 totalRead = 0;

        static const int bufsize = 4095;
        char buf[bufsize+1];

        int iline = 0;
        while(!ff.atEnd())
        {
            int sizeRead = ff.readLine(buf,bufsize);
            ++iline;
            totalRead += sizeRead;
            emitProgress(totalRead,fileSize);
            if (sizeRead >= bufsize)
            {
                if (mErrorCount[5]++ < maxErrorMessages)
                    log_warning("waveform_viewer", "VCD line {} exceeds buffer size {}.", iline, bufsize);
                return false;
            }

            if (sizeRead < 0)
            {
                if (mErrorCount[6]++ < maxErrorMessages)
                    log_warning("waveform_viewer", "VCD parse error reading line {} from file '{}'.", iline, ff.fileName().toStdString());
                return false;
            }
            if (sizeRead > 0 && buf[sizeRead-1]=='\n') --sizeRead;
            if (sizeRead > 0 && buf[sizeRead-1]=='\r') --sizeRead;
            if (!sizeRead) continue;


            if (parseHeader)
            {
                QByteArray line(buf,sizeRead);
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
                        const Net* net = netNames.value(wireName);
                        if (!netNames.isEmpty() && !net) continue; // net not found in given name list
                        if (mAbbrevByName.contains(wireName))
                        {
                            if (mErrorCount[7]++ < maxErrorMessages)
                                log_warning("waveform_viewer", "Waveform duplicate for '{}' in VCD file '{}'.", wireName.toStdString(), ff.fileName().toStdString());
                            continue;
                        }
                        QString wireAbbrev = mWire.captured(2);
                        mAbbrevByName.insert(wireName,wireAbbrev);
                        int     wireBits   = mWire.captured(1).toUInt(&ok);
                        if (!ok) wireBits = 1;
                        if (wireBits > 1) continue; // TODO : decision whether we will be able to handle VCD with more bits

                        u32 netId = net ? net->get_id() : 0;

                        SaleaeOutputFile* sof = nullptr;
                        if (mSaleaeFiles.contains(wireAbbrev))
                        {
                            // output file already exists, need name entry
                            sof = mSaleaeFiles.value(wireAbbrev);
                            if (sof) mSaleaeWriter->add_directory_entry(sof->index(), wireName.toStdString(), netId);
                        }
                        else
                        {
                            sof = mSaleaeWriter->add_or_replace_waveform(wireName.toStdString(), netId);
                            if (sof) mSaleaeFiles.insert(wireAbbrev,sof);
                        }
                    }
                }
            }
            else
            {
                if (!parseVcdDataline(buf,sizeRead))
                {
                    if (mErrorCount[8]++ < maxErrorMessages)
                        log_warning("waveform_viewer", "Cannot parse VCD data line '{}'.", QByteArray(buf,sizeRead));
                    return false;
                }
            }
        }
        return true;
    }

    bool VcdSerializer::importSaleae(const QString& saleaeDirecotry, const std::unordered_map<hal::Net*, int> &lookupTable, const QString& workdir, u64 timeScale)
    {
        mWorkdir = workdir.isEmpty() ? QDir::currentPath() : workdir;
        qDebug() << "workdir" << mWorkdir;
        deleteFiles();
        mTime = 0;
        SaleaeParser::sTimeScaleFactor = timeScale;
        int nstep = lookupTable.size() + 1;
        int istep = 0;

        emitProgress(istep++,nstep);
        createSaleaeDirectory();
        SaleaeDirectory sd(get_saleae_directory_filename());
        SaleaeDirectoryStoreRequest save(&sd);
        QDir sourceDir(saleaeDirecotry);
        QDir targetDir(QFileInfo(mSaleaeDirectoryFilename).path());
        emitProgress(istep++,nstep);

        for (auto it = lookupTable.begin(); it != lookupTable.end(); ++it)
        {
            Q_ASSERT(it->first);
            bool removeOldFile = false;
            int inx = sd.get_datafile_index(it->first->get_name(),it->first->get_id());
            if (inx < 0)
                // create new file in import direcotry
                inx = sd.get_next_available_index();
            else
                removeOldFile = true;
            QString source = sourceDir.absoluteFilePath(QString("digital_%1.bin").arg(it->second));
            QString target = targetDir.absoluteFilePath(QString("digital_%1.bin").arg(inx));
            if (removeOldFile)
                QFile::remove(target);
            if (!QFile::copy(source,target)) return false;
            SaleaeInputFile sif(target.toStdString());
            if (!sif.header()) return false;
            SaleaeDirectoryNetEntry sdne(it->first->get_name(), it->first->get_id());
            sdne.addIndex(SaleaeDirectoryFileIndex(inx,sif.header()->beginTime(),sif.header()->endTime(),sif.header()->numTransitions()+1));
            sd.add_or_replace_net(sdne);
            emitProgress(istep++,nstep);
        }
        emitImportDone();
        return true;
    }
}
