#include "gui/comment_system/comment_manager.h"
#include "hal_core/netlist/project_manager.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace hal
{

    CommentManager::CommentManager(QObject* parent)
        : QObject(parent), ProjectSerializer("comments")
    {;}

    CommentManager::~CommentManager()
    {
        clear();
    }

    std::string CommentManager::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        Q_UNUSED(isAutosave);
        QString comFilename("comments.json");
        QFile comFile(QDir(QString::fromStdString(savedir.string())).absoluteFilePath(comFilename));
        if (!comFile.open(QIODevice::WriteOnly)) return std::string();

        QJsonObject comObj;
        QJsonArray  comArr;

        for (auto it = mEntries.constBegin(); it != mEntries.constEnd(); ++it)
        {
            const CommentEntry* comEnt = it.value();
            Q_ASSERT(comEnt);
            comArr.append(comEnt->toJson());
        }

        comObj["comments"] = comArr;

        comFile.write(QJsonDocument(comObj).toJson(QJsonDocument::Compact));

        return comFilename.toStdString();
    }

    void CommentManager::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (!relname.empty())
            restoreInternal(loaddir, relname);
    }

    bool CommentManager::restore()
    {
        ProjectManager* pm = ProjectManager::instance();
        std::string relname = pm->get_filename(m_name);
        if (relname.empty()) return false;

        restoreInternal(pm->get_project_directory(), relname);
        return false;
    }

    bool CommentManager::restoreInternal(const std::filesystem::path& loaddir, const std::string& relFilename)
    {
        QFile comFile(QDir(QString::fromStdString(loaddir.string())).absoluteFilePath(QString::fromStdString(relFilename)));
        if (!comFile.open(QIODevice::ReadOnly))
            return false;
        QJsonParseError parseStatus;
        QJsonDocument jsonDoc   = QJsonDocument::fromJson(comFile.readAll(),&parseStatus);
        if (parseStatus.error != QJsonParseError::NoError)
            return false;
        const QJsonObject& json = jsonDoc.object();

        if (json.contains("comments") && json["comments"].isArray())
        {
            QJsonArray comArr = json["comments"].toArray();
            int nCom          = comArr.size();
            for (int iCom = 0; iCom < nCom; iCom++)
            {
                QJsonObject comElem = comArr.at(iCom).toObject();
                CommentEntry* comEntry = new CommentEntry(comElem);
                if (comEntry->isInvalid())
                {
                    delete comEntry;
                    continue;
                }
                mEntries.insertMulti(comEntry->getNode(),comEntry);
            }
        }

        // for developer: dump entries
        for (const CommentEntry* ce : mEntries.values())
        {
            ce->dump();
        }

        return true;
    }

    void CommentManager::clear()
    {
        for (CommentEntry* ce : mEntries)
            delete ce;
        mEntries.clear();
    }

    QList<CommentEntry*> CommentManager::getEntriesForNode(const Node& nd) const
    {
//        QDateTime now = QDateTime::currentDateTime();
//        QMap<qint64,CommentEntry*> entriesFound;
//        auto it = mEntries.find(nd);
//        while (it != mEntries.end() && it.key() == nd)
//        {
//            entriesFound.insert(it.value()->getCreationTime().msecsTo(now),it.value());
//            ++it;
//        }
//        return entriesFound.values();
        return mEntries.values(nd);
    }

    bool CommentManager::contains(const Node& nd) const
    {
        return mEntries.contains(nd);
    }

    void CommentManager::deleteComment(CommentEntry *entry)
    {
        auto list = mEntries.values(entry->getNode());
        if(!list.contains(entry)) // in case someone created an entry outside of the CommentManager context
            return;

        // all observer must finish handling the signal, otherwise big problems arise when the entry
        // is deleted here. If the above signal is asynchronous, the entry cannot but used to identify
        // the comment, but rather an id that is stored in the comment widgets and items seperately.
        mEntries.remove(entry->getNode(), entry); // check if this only removes the entry, not the entire node

        Q_EMIT entryAboutToBeDeleted(entry);

        delete entry;
    }

    void CommentManager::addComment(CommentEntry *entry)
    {
        // sanity check, dont add same entry twice
        for(const auto &item : mEntries.values(entry->getNode()))
            if(item == entry)  return;

        mEntries.insertMulti(entry->getNode(), entry);
        Q_EMIT entryAdded(entry);
    }

    void CommentManager::relayEntryModified(CommentEntry *entry)
    {
        Q_EMIT entryModified(entry);
    }

}
