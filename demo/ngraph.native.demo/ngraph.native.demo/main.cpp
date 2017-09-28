// ngraph2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "application.h"

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
        std::wcout << L"[Error] base_exception = " << e.get_message() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << L"[Error] std::exception = " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << L"[Critical] Unknown exception" << std::endl;
    }

    return nExitCode;
}
