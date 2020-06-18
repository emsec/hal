#include <QDir>
#include <fstream>
#include <gui/python/python_console_history.h>

#include "python/python_console_history.h"
namespace hal{
python_console_history::python_console_history() : m_file_name(QDir::home().filePath(".hal_history").toLocal8Bit().constData()), m_history()
{
    update_from_file();
}

python_console_history::~python_console_history()
{
}

void python_console_history::add_history(const std::string& command)
{
    m_history.push_back(command);
    append_to_file(command);
}

const std::string& python_console_history::get_history_item(const int& index) const
{
    return m_history.at(index);
}

int python_console_history::size() const
{
    return m_history.size();
}

void python_console_history::update_from_file()
{
    std::ifstream f(m_file_name);
    m_history.clear();
    std::string line;
    while (std::getline(f, line))
    {
        m_history.push_back(line);
    }
}

void python_console_history::append_to_file(const std::string& command)
{
    std::ofstream f(m_file_name, std::ios::out | std::ios::app);
    f << command << std::endl;
}
}
