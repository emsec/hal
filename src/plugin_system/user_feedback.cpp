#include "hal_core/plugin_system/user_feedback.h"

#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>

namespace hal
{
    namespace user_feedback
    {
        namespace
        {
            /** A terminal wider than this is not a terminal, so do not try to draw a progress bar across it. */
            constexpr int MAX_PLAUSIBLE_TERMINAL_WIDTH = 4096;

            /** Below this width there is no room for a bar next to the percentage. */
            constexpr int MIN_TERMINAL_WIDTH = 8;
        }    // namespace

        void report_progress(int percent, const std::string& message)
        {
            if (auto* ui_plugin = plugin_manager::get_ui_plugin(); ui_plugin != nullptr)
            {
                ui_plugin->set_progress(percent, message);
            }
        }

        LayoutLocker::LayoutLocker() : m_ui_plugin(plugin_manager::get_ui_plugin())
        {
            if (m_ui_plugin != nullptr)
            {
                m_ui_plugin->set_layout_locker(true);
            }
        }

        LayoutLocker::~LayoutLocker()
        {
            if (m_ui_plugin != nullptr)
            {
                m_ui_plugin->set_layout_locker(false);
            }
        }

        std::atomic<u32> ProgressScope::s_depth(0);

        ProgressScope::ProgressScope(const std::string& message)
        {
            if (s_depth++ == 0)
            {
                report_progress(0, message);
            }
        }

        ProgressScope::~ProgressScope()
        {
            // the progress display treats 100 percent as "done" and dismisses itself, so it must be reported exactly
            // once per operation
            if (--s_depth == 0)
            {
                report_progress(100, "done");
            }
        }

        ProgressPrinter::ProgressPrinter(const std::string& message, u32 max_detail_size)
            : m_scope(message), m_message(message), m_printed_progress(0), m_bar_width(0), m_max_detail_size(max_detail_size), m_last_percentage(0), m_terminal_width(get_terminal_width())
        {
            if (m_terminal_width > MIN_TERMINAL_WIDTH)
            {
                // the bar shares the line with "[] 100%" and, if one was asked for, a detail
                m_bar_width = m_terminal_width - 7;
                if (max_detail_size != 0)
                {
                    m_bar_width -= max_detail_size + 1;
                }
            }

            reset();
        }

        ProgressPrinter::~ProgressPrinter()
        {
            clear();
        }

        void ProgressPrinter::set_message(const std::string& message)
        {
            m_message = message;
            report_progress(m_last_percentage, m_message);
        }

        void ProgressPrinter::report(float progress, const std::string& detail)
        {
            progress          = std::clamp(progress, 0.0f, 1.0f);
            m_last_percentage = (int)(progress * 100.0f);

            // 100 percent dismisses the progress display of the user interface, which is up to the ProgressScope
            // bracketing the operation rather than to a report from within it
            report_progress(std::min(m_last_percentage, 99), m_message);

            if (m_terminal_width <= MIN_TERMINAL_WIDTH)
            {
                return;
            }

            const u32 pos = (u32)(m_bar_width * progress);
            if (pos <= m_printed_progress && m_last_detail == detail)
            {
                // nothing about the bar would change, so leave the terminal alone
                return;
            }

            m_last_detail      = detail;
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

            auto printed_detail = detail;
            if (printed_detail.size() > m_max_detail_size)
            {
                printed_detail = (m_max_detail_size <= 3) ? std::string(m_max_detail_size, '.') : printed_detail.substr(0, m_max_detail_size - 3) + "...";
            }

            std::stringstream str;
            str << "[" << bar << "] " << std::right << std::setw(3) << m_last_percentage << '%';
            if (!printed_detail.empty())
            {
                str << " " << printed_detail;
            }

            const auto output = str.str();
            std::cerr << output;
            if (output.size() < (u32)m_terminal_width)
            {
                std::cerr << std::string(m_terminal_width - output.size(), ' ');
            }
            std::cerr << "\r" << std::flush;
        }

        void ProgressPrinter::clear()
        {
            if (m_terminal_width <= MIN_TERMINAL_WIDTH)
            {
                return;
            }
            std::cerr << std::string(m_terminal_width, ' ') << "\r" << std::flush;
        }

        void ProgressPrinter::reset()
        {
            m_printed_progress = 0;

            // no detail can ever compare equal to this, so the next report draws the bar in any case
            m_last_detail = std::string("\0redraw", 7);
        }

        int ProgressPrinter::get_terminal_width()
        {
            struct winsize size;
            if (ioctl(STDERR_FILENO, TIOCGWINSZ, &size) < 0)
            {
                return -1;
            }
            if (size.ws_col > MAX_PLAUSIBLE_TERMINAL_WIDTH)
            {
                return -1;
            }
            return size.ws_col;
        }
    }    // namespace user_feedback
}    // namespace hal
