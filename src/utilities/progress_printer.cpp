#include "hal_core/utilities/progress_printer.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>    //ioctl() and TIOCGWINSZ
#include <unistd.h>       // for STDOUT_FILENO

const int dont_believe_in_terminal_width_bigger_than_this_value = 4096;

namespace hal
{
    ProgressPrinter::ProgressPrinter(u32 max_message_size) : m_bar_width(0), m_max_message_size(max_message_size), m_terminal_width(get_terminal_width())
    {
        if (m_terminal_width <= 8)
        {
            return;
        }

        m_bar_width = m_terminal_width - 7;
        if (max_message_size != 0)
        {
            m_bar_width -= max_message_size + 1;
        }
        reset();
    }

    void ProgressPrinter::print_progress(float progress, const std::string& message)
    {
        progress         = std::clamp(progress, 0.f, 1.f);
        u32 int_progress = (u32)(progress * 100.0f);

        if (m_terminal_width <= 8)
        {
            return;
        }

        u32 pos = (u32)(m_bar_width * progress);

        if (pos <= m_printed_progress && m_last_message == message)
        {
            return;
        }

        m_last_message     = message;
        m_printed_progress = pos;

        std::string bar;
        if (pos)
        {
            bar += std::string(pos, '=');
        }
        if (pos < m_bar_width)
        {
            bar += '>';
        }
        if (pos + 1 < m_bar_width)
        {
            bar += std::string(m_bar_width - pos - 1, ' ');
        }

        auto print_message = message;
        if (print_message.size() > m_max_message_size)
        {
            if (m_max_message_size <= 3)
            {
                print_message = std::string(m_max_message_size, '.');
            }
            else
            {
                print_message = print_message.substr(0, m_max_message_size - 3) + "...";
            }
        }

        std::stringstream str;
        str << "[" << bar << "] ";

        str << std::right << std::setw(3) << int_progress << '%';

        if (!print_message.empty())
        {
            str << " " << print_message;
        }

        auto output = str.str();

        std::cerr << output;
        if (output.size() < (u32)m_terminal_width)
        {
            std::cerr << std::string(m_terminal_width - output.size(), ' ');
        }

        std::cerr << "\r" << std::flush;
    }

    void ProgressPrinter::clear()
    {
        if (m_terminal_width <= 8)
        {
            return;
        }
        std::cerr << std::string(m_terminal_width, ' ') << "\r" << std::flush;
    }

    void ProgressPrinter::reset()
    {
        m_printed_progress = 0;
        m_last_message     = "$^äü+";
    }

    int ProgressPrinter::get_terminal_width()
    {
        struct winsize size;
        int retval = ioctl(STDERR_FILENO, TIOCGWINSZ, &size);
        if (retval < 0)
        {
            return -1;
        }
        if (size.ws_col > dont_believe_in_terminal_width_bigger_than_this_value)
        {
            return -1;
        }
        return size.ws_col;
    }

}    // namespace hal
