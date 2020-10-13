#pragma once

#include "hal_core/defines.h"

#include <cmath>
#include <functional>
#include <set>
#include <thread>

namespace hal
{
    namespace dataflow
    {
        namespace utils
        {
            template<typename R>
            void parallel_for_each(u32 begin, u32 end, R func)
            {
                if (end - begin == 0)
                {
                    return;
                }

                u32 num_threads      = std::min(end - begin, std::thread::hardware_concurrency());
                u32 steps_per_thread = std::ceil((float)(end - begin) / num_threads);

                std::vector<std::thread> threads;
                threads.reserve(num_threads);

                // spawn threads, start at index 1 since index 0 is the main thread
                for (u32 thread_idx = 1; thread_idx < num_threads; ++thread_idx)
                {
                    threads.emplace_back([&func, end, thread_idx, steps_per_thread]() {
                        u32 local_end = std::min(end, steps_per_thread * (thread_idx + 1));
                        for (u32 i = steps_per_thread * thread_idx; i < local_end; ++i)
                        {
                            func(i);
                        }
                    });
                }

                // also do work on main thread
                {
                    u32 local_end = std::min(end, steps_per_thread);
                    for (u32 i = 0; i < local_end; ++i)
                    {
                        func(i);
                    }
                }

                // wait until all threads are done
                for (auto& t : threads)
                {
                    t.join();
                }
            }

            template<typename T, typename R>
            void parallel_for_each(const std::vector<T>& elements, R func)
            {
                parallel_for_each(0, elements.size(), [&elements, &func](u32 i) { func(elements[i]); });
            }

            template<typename T, typename R>
            void parallel_for_each(const std::set<T>& elements, R func)
            {
                std::vector<T> vec;
                vec.reserve(elements.size());
                vec.insert(vec.end(), elements.begin(), elements.end());
                parallel_for_each(vec, func);
            }

            template<typename T, typename R>
            void parallel_for_each(const std::unordered_set<T>& elements, R func)
            {
                std::vector<T> vec;
                vec.reserve(elements.size());
                vec.insert(vec.end(), elements.begin(), elements.end());
                parallel_for_each(vec, func);
            }
        }    // namespace utils
    }        // namespace dataflow
}