#include <QDir>
#include <fstream>
#include <gui/python/python_console_history.h>

#include "gui/python/python_console_history.h"

namespace hal
{
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
            mHistory.push_back(line);
        }
    }

    void PythonConsoleHistory::appendToFile(const std::string& command)
    {
        std::ofstream f(mFileName, std::ios::out | std::ios::app);
        f << command << std::endl;
    }
}
