#include <QDir>
#include <fstream>
#include <gui/python/python_console_history.h>

#include "gui/python/python_console_history.h"

namespace hal
{
    namespace
    {
        // one history entry per line in file, line breaks within multi-line commands are stored as U+2029 (UTF-8)
        const std::string sFileLineBreak = "\xE2\x80\xA9";

        std::string replaceAll(std::string text, const std::string& from, const std::string& to)
        {
            for (size_t pos = text.find(from); pos != std::string::npos; pos = text.find(from, pos + to.size()))
                text.replace(pos, from.size(), to);
            return text;
        }
    }

    PythonConsoleHistory::PythonConsoleHistory() : mFileName(QDir::home().filePath(".hal_history").toLocal8Bit().constData()), mHistory()
    {
        updateFromFile();
    }

    PythonConsoleHistory::~PythonConsoleHistory()
    {
    }

    void PythonConsoleHistory::addHistory(const std::string& command)
    {
        mHistory.push_back(command);
        appendToFile(command);
    }

    const std::string& PythonConsoleHistory::getHistoryItem(const int& index) const
    {
        return mHistory.at(index);
    }

    int PythonConsoleHistory::size() const
    {
        return mHistory.size();
    }

    void PythonConsoleHistory::updateFromFile()
    {
        std::ifstream f(mFileName);
        mHistory.clear();
        std::string line;
        while (std::getline(f, line))
        {
            mHistory.push_back(replaceAll(line, sFileLineBreak, "\n"));
        }
    }

    void PythonConsoleHistory::appendToFile(const std::string& command)
    {
        std::ofstream f(mFileName, std::ios::out | std::ios::app);
        f << replaceAll(command, "\n", sFileLineBreak) << std::endl;
    }
}
