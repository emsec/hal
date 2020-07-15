#pragma once

#include "core/log.h"

#include <chrono>

#define MERGE_(a, b) a##b
#define LABEL_(a) MERGE_(measure_block_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

#define measure_block_time(X) measure_block_time_t UNIQUE_NAME(X);

#define seconds_since(X) ((double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - (X)).count() / 1000)

namespace hal
{
    class measure_block_time_t
    {
    public:
        measure_block_time_t(const std::string& section_name)
        {
            m_name       = section_name;
            m_begin_time = std::chrono::high_resolution_clock::now();
        }

        ~measure_block_time_t()
        {
            log_info("dataflow", "{} took {:3.2f}s", m_name, seconds_since(m_begin_time));
        }

    private:
        std::string m_name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_begin_time;
    };
}    // namespace hal