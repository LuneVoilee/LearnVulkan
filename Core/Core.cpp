#include <exception>
#include <iostream>
#include <ostream>
#include "MainLoop.h"

int main()
{
    std::cout << _MSVC_LANG << std::endl; //202002
    HelloTriangleApplication app;
    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
