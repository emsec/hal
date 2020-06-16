#include "core/program_arguments.h"

#include "core/log.h"

namespace hal
{
    ProgramArguments::ProgramArguments()
    {
        m_argc = 0;
        m_argv = nullptr;
    }

    ProgramArguments::ProgramArguments(int argc, const char** argv)
    {
        m_argc = argc;
        m_argv = argv;
    }

    void ProgramArguments::get_original_arguments(int* argc, const char*** argv)
    {
        *argc = m_argc;
        *argv = m_argv;
    }

    std::vector<std::string> ProgramArguments::get_set_options() const
    {
        return m_given_flags;
    }

    bool ProgramArguments::is_option_set(const std::string& flag) const
    {
        // checks all groups of equivalent flags too
        for (const auto& it : m_set_options)
        {
            if (it.first.find(flag) != it.first.end())
            {
                return true;
            }
        }
        return false;
    }

    void ProgramArguments::set_option(const std::string& flag, const std::vector<std::string>& parameters)
    {
        set_option(flag, {}, parameters);
    }

    bool ProgramArguments::set_option(const std::string& flag, const std::set<std::string>& equivalent_flags, const std::vector<std::string>& parameters)
    {
        std::set<std::string> option_flags = equivalent_flags;
        option_flags.insert(flag);

        // remember the first matching option here (if it exists)
        auto it_matching_option = m_set_options.end();

        // "count" options that contain any of the given flags here
        std::set<const std::set<std::string>*> options_containing_flags;

        for (auto it = m_set_options.begin(); it != m_set_options.end(); ++it)
        {
            for (const auto& f : option_flags)
            {
                if (it->first.find(f) != it->first.end())
                {
                    options_containing_flags.insert(&(it->first));
                    it_matching_option = it;
                }
            }
        }

        // if the flags were split over multiple options --> error
        if (options_containing_flags.size() > 1)
        {
            log_error("core",
                      "Given flags ({}) are parts of {} different options.",
                      core_utils::join(", ", std::vector<std::string>(option_flags.begin(), option_flags.end())),
                      options_containing_flags.size());
            return false;
        }
        // if the flags are found in a single option, 'it_matching_option' points to it
        else if (options_containing_flags.size() == 1)
        {
            // if all given flags are not a subset of the already known flags --> error
            if (!std::includes(it_matching_option->first.begin(), it_matching_option->first.end(), option_flags.begin(), option_flags.end()))
            {
                log_error("core",
                          "Some flags of ({}) are already set, but not all given flags are flags for that option.",
                          core_utils::join(", ", std::vector<std::string>(option_flags.begin(), option_flags.end())));
                return false;
            }
        }

        // overwrite an existing flag (keeping the old flag list as the new one has to be a subset), or register a new one
        if (it_matching_option != m_set_options.end())
        {
            it_matching_option->second = parameters;
        }
        else
        {
            m_set_options[option_flags] = parameters;
        }

        if (std::find(m_given_flags.begin(), m_given_flags.end(), flag) == m_given_flags.end())
        {
            m_given_flags.push_back(flag);
        }

        return true;
    }

    std::string ProgramArguments::get_parameter(const std::string& flag) const
    {
        auto list = get_parameters(flag);
        if (list.empty())
        {
            return "";
        }
        return list[0];
    }

    std::vector<std::string> ProgramArguments::get_parameters(const std::string& flag) const
    {
        for (const auto& it : m_set_options)
        {
            if (it.first.find(flag) != it.first.end())
            {
                return it.second;
            }
        }

        log_error("core", "The flag '{}' is not registered.", flag);
        return {};
    }
}    // namespace hal
