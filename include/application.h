#pragma once

class CApplication
{
public:
    int run(
        const int argc,
        const char* argv[]);

private:
    static size_t get_iteration_start_from_name(
        const fs::path& pathPositionFileName);

private:
    bool parse_options(
        const int argc,
        const char* argv[]);

    void print_help();
};
