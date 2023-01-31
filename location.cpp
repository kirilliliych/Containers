#include "location.hpp"

void print_location(const Location &location)
{
    std::cout << "__FILE__: " << location.file_ << std::endl;
    std::cout << "__FUNC__: " << location.func_ << std::endl;
    std::cout << "__LINE__: " << location.line_ << std::endl << std::endl;
}