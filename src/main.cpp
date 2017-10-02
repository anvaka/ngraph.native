#include <stdafx.h>

#include <application.h>

int main(const int argc, const char* argv[])
{
    int nExitCode = EXIT_FAILURE;

    try
    {
        CApplication app;
        nExitCode = app.run(argc, argv);
    }
    catch (const base_exception& e)
    {
        std::cout << "[Error] base_exception = " << e.get_message() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[Error] std::exception = " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "[Critical] Unknown exception" << std::endl;
    }

    return nExitCode;
}
