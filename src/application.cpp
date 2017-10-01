#include "stdafx.h"

#include <regex>

#include "layout.h"

#include "application.h"
#include "application_settings.h"


int CApplication::run(const int argc, const char * argv[])
{
    using namespace std::chrono;

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

        time_point<steady_clock> start, start_iteration, end;
        milliseconds time_min = 0ms, time_max = 0ms, time_avg = 0ms;

        start = high_resolution_clock::now();
        CLayout layout;
        end = high_resolution_clock::now();

        if (g_Settings.is_verbose())
        {
            std::wcout
                << L"=========" << std::endl;
            std::wcout
                << L"Start iteration = " << nStartIteration << std::endl
                << L"Maximum iterations = " << g_Settings.get_max_iteration() << std::endl
                << L"Count vertex in graph = " << layout.get_bodies().size() << std::endl;
            std::wcout
                << L"=========" << std::endl;
            std::wcout
                << L"Time spent on load layout = "
                << std::chrono::duration_cast<milliseconds>(end - start).count()
                << L"ms\n";
            std::wcout
                << L"=========" << std::endl;
        }

        size_t i = nStartIteration;
        size_t nIntervalSave = g_Settings.get_interval_save();
        start_iteration = high_resolution_clock::now();
        for (size_t max = g_Settings.get_max_iteration(); i < max; ++i)
        {
            start = high_resolution_clock::now();

            bool bCompleted = layout.step();

            end = high_resolution_clock::now();

            if (g_Settings.is_verbose())
            {
                const auto diff = duration_cast<milliseconds>(end - start);
                if (time_min == 0ms || diff < time_min)
                    time_min = diff;
                else if (diff > time_max)
                    time_max = diff;

                time_avg += diff;

                std::wcout
                    << L"=========" << std::endl;
                std::wcout
                    << L"Time spent on step(" << i << L") = "
                    << diff.count()
                    << L"ms\n";
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
            end = high_resolution_clock::now();
            std::wcout
                << L"=========" << std::endl;
            std::wcout
                << L"Total time spent on iterations(" << i << L") = "
                << duration_cast<milliseconds>(end - start_iteration).count()
                << L"ms\n";
            std::wcout
                << L"=========" << std::endl;
            std::wcout
                << L"min(" << time_min.count() << L"ms), "
                << L"max(" << time_max.count() << L"ms), "
                << L"avg(" << time_avg.count() / (i + 1) << L"ms)\n";
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
    static const wchar_t* wszErrorMessage = L"Invalid argument. ";

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
        else if (arg == "-l" || arg == "--links")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION(wszErrorMessage << L"Need arg for 'links' option");

            g_Settings.set_links_file(args[++i]);
        }
        else if (arg == "-p" || arg == "--positions")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION(wszErrorMessage << L"Need arg for 'positions' option");

            g_Settings.set_positions_file(args[++i]);
        }
        else if (arg == "-w" || arg == "--weights")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION(wszErrorMessage << L"Need arg for 'weights' option");

            g_Settings.set_weights_file(args[++i]);
        }
        else if (arg == "-s" || arg == "--save")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION(wszErrorMessage << L"Need arg for 'save' option");

            g_Settings.set_save_positions_file(args[++i]);
        }
        else if (arg == "-i" || arg == "--save_interval")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION(wszErrorMessage << L"Need arg for 'save_interval' option");

            g_Settings.set_interval_save(std::stoi(args[++i]));
        }
        else if (arg == "-m" || arg == "--max_iterations")
        {
            if (i + 1 >= args.size())
                THROW_EXCEPTION(wszErrorMessage << L"Need arg for 'max_iterations' option");

            g_Settings.set_max_iteration(std::stoi(args[++i]));
        }
    }

    return true;
}

void CApplication::print_help()
{
    std::wcout <<
        L"Allowed options:\n"
        L"  -h, --help              Show this message\n"
        L"  -v, --verbose           Verbose mode\n"
        L"  -s, --save              Path to file for save result positions\n"
        L"  -l, --links             Path to file with serialized graph\n"
        L"                          See https://github.com/anvaka/ngraph.tobinary for format description\n"
        L"  -p, --positions         Path to file with positions for vertices. Next step started with this snapshot\n"
        L"  -w, --weights           Path to file with weights for vertices\n"
        L"  -i, --save_interval     Save snapshot position each interval iterations\n"
        L"  -m, --max_iterations    Maximum iterations for algorithm\n";
}
