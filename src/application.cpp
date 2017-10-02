#include <stdafx.h>

#include <regex>

#include <layout.h>

#include <application.h>
#include <application_settings.h>


int CApplication::run(const int argc, const char * argv[])
{
    int nExitCode = EXIT_FAILURE;

    do
    {
        if (!parse_options(argc, argv))
        {
            nExitCode = EXIT_SUCCESS;
            break;
        }

        size_t nStartIteration = 0;
        if (g_Settings.is_required_positions_file())
        {
            nStartIteration = get_iteration_start_from_name(
                g_Settings.get_positions_file());
        }

        std::chrono::time_point<std::chrono::system_clock> start, start_iteration, end;
        std::chrono::milliseconds time_min(0), time_max(0), time_avg(0);

        start = std::chrono::system_clock::now();
        CLayout layout;
        end = std::chrono::system_clock::now();

        if (g_Settings.is_verbose())
        {
            std::cout
                << "=========" << std::endl;
            std::cout
                << "Start iteration = " << nStartIteration << std::endl
                << "Maximum iterations = " << g_Settings.get_max_iteration() << std::endl
                << "Count vertex in graph = " << layout.get_bodies().size() << std::endl;
            std::cout
                << "=========" << std::endl;
            std::cout
                << "Time spent on load layout = "
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                << "ms\n";
            std::cout
                << "=========" << std::endl;
        }

        size_t i = nStartIteration;
        size_t nIntervalSave = g_Settings.get_interval_save();
        start_iteration = std::chrono::system_clock::now();
        for (size_t max = g_Settings.get_max_iteration(); i < max; ++i)
        {
            start = std::chrono::system_clock::now();

            bool bCompleted = layout.step();

            end = std::chrono::system_clock::now();

            if (g_Settings.is_verbose())
            {
                const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                if (time_min == std::chrono::milliseconds(0) || diff < time_min)
                    time_min = diff;
                else if (diff > time_max)
                    time_max = diff;

                time_avg += diff;

                std::cout
                    << "=========" << std::endl;
                std::cout
                    << "Time spent on step(" << i << ") = "
                    << diff.count()
                    << "ms\n";
            }

            if (bCompleted)
            {
                break;
            }

            if (nIntervalSave > 0 && i % nIntervalSave == 0)
            {
                layout.serialize_to_file(i);
            }
        }

        if (g_Settings.is_verbose())
        {
            end = std::chrono::system_clock::now();
            std::cout
                << "=========" << std::endl;
            std::cout
                << "Total time spent on iterations(" << i << ") = "
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_iteration).count()
                << "ms\n";
            std::cout
                << "=========" << std::endl;
            std::cout
                << "min(" << time_min.count() << "ms), "
                << "max(" << time_max.count() << "ms), "
                << "avg(" << time_avg.count() / (i + 1) << "ms)\n";
        }

        layout.serialize_to_file();

        nExitCode = EXIT_SUCCESS;
    } while (false);

    return nExitCode;
}

size_t CApplication::get_iteration_start_from_name(
    const fs::path & pathPositionFileName)
{
    size_t position = 0;

    do
    {
        std::cmatch match;
        std::regex pattern(".*?positions_(\\d+)\\.bin$");
        std::string sFileName = pathPositionFileName.string();
        if (!std::regex_match(sFileName.c_str(), match, pattern))
        {
            break;
        }

        if (match.size() < 2)
        {
            break;
        }

        position = std::stoull(match[1]) + 1;
    } while (false);

    return position;
}

bool CApplication::parse_options(const int argc, const char * argv[])
{
    std::vector<std::string> args(&argv[1], &argv[argc]);
    for (int i = 0; i < args.size(); ++i)
    {
        const auto& arg = args[i];

        if (arg == "-h" || arg == "--help")
        {
            print_help();
            return false;
        }
        else if (arg == "-v" || arg == "--verbose")
        {
            g_Settings.set_verbose(true);
        }
        else if (arg == "-" || arg == "--links")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION("Invalid argument. " << "Need arg for 'links' option");

            g_Settings.set_links_file(args[++i]);
        }
        else if (arg == "-p" || arg == "--positions")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION("Invalid argument. " << "Need arg for 'positions' option");

            g_Settings.set_positions_file(args[++i]);
        }
        else if (arg == "-w" || arg == "--weights")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION("Invalid argument. " << "Need arg for 'weights' option");

            g_Settings.set_weights_file(args[++i]);
        }
        else if (arg == "-s" || arg == "--save")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION("Invalid argument. " << "Need arg for 'save' option");

            g_Settings.set_save_positions_file(args[++i]);
        }
        else if (arg == "-i" || arg == "--save_interva")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION("Invalid argument. " << "Need arg for 'save_interval' option");

            g_Settings.set_interval_save(std::stoi(args[++i]));
        }
        else if (arg == "-m" || arg == "--max_iterations")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION("Invalid argument. " << "Need arg for 'max_iterations' option");

            g_Settings.set_max_iteration(std::stoi(args[++i]));
        }
    }

    return true;
}

void CApplication::print_help()
{
    std::cout <<
        "Allowed options:\n"
        "  -h, --help              Show this message\n"
        "  -v, --verbose           Verbose mode\n"
        "  -s, --save              Path to file for save result positions\n"
        "  -l, --links             Path to file with serialized graph\n"
        "                          See https://github.com/anvaka/ngraph.tobinary for format description\n"
        "  -p, --positions         Path to file with positions for vertices. Next step started with this snapshot\n"
        "  -w, --weights           Path to file with weights for vertices\n"
        "  -i, --save_interval     Save snapshot position each interval iterations\n"
        "  -m, --max_iterations    Maximum iterations for algorithm\n";
}
