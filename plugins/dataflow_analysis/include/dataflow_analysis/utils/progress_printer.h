#pragma once

#include "hal_core/defines.h"

#include <iostream>
#include <sys/ioctl.h>    //ioctl() and TIOCGWINSZ
#include <unistd.h>       // for STDOUT_FILENO

namespace hal
{
    class progress_printer
    {
    public:
        progress_printer(u32 max_message_size = 0)
        {
            m_max_message_size = max_message_size;
            m_terminal_width   = get_terminal_width();
            m_bar_width        = m_terminal_width - 7;
            if (max_message_size != 0)
            {
                m_bar_width -= max_message_size + 1;
            }
            reset();
        }

        void print_progress(float progress, const std::string& message = "")
        {
            u32 pos = std::clamp((u32)(m_bar_width * progress), (u32)0, m_bar_width);

            if (pos <= m_printed_progress && m_last_message == message)
            {
                return;
            }

            m_last_message     = message;
            m_printed_progress = pos;

            std::stringstream str;
            str << "[";
            for (u32 i = 0; i < m_bar_width; ++i)
            {
                if (i < pos)
                {
                    str << "=";
                }
                else if (i == pos)
                {
                    str << ">";
                }
                else
                {
                    str << " ";
                }
            }

            auto print_message = message;
            if (print_message.size() > m_max_message_size)
            {
                print_message = print_message.substr(0, m_max_message_size - 3) + "...";
            }
            str << "] ";
            u32 int_progress = (u32)(progress * 100.0f);
            if (int_progress < 10)
            {
                str << " ";
            }
            if (int_progress < 100)
            {
                str << " ";
            }
            str << int_progress << "%";

            if (!print_message.empty())
            {
                str << " " << print_message;
            }

            auto output = str.str();

            std::cerr << output;
            for (u32 i = 0; i < m_terminal_width - output.size(); ++i)
            {
                std::cerr << " ";
            }
            std::cerr << "\r" << std::flush;
        }

        void clear()
        {
            for (u32 i = 0; i < m_terminal_width; ++i)
            {
                std::cerr << " ";
            }
            std::cerr << "\r" << std::flush;
        }

        void reset()
        {
            m_printed_progress = 0;
            m_last_message     = "$^äü+";
        }

    private:
        u32 get_terminal_width()
        {
            struct winsize size;
            ioctl(STDERR_FILENO, TIOCGWINSZ, &size);
            return size.ws_col;
        }

        u32 m_printed_progress;
        std::string m_last_message;
        u32 m_bar_width;
        u32 m_max_message_size;
        u32 m_terminal_width;
    };
}    // namespace hal
