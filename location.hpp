#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>

struct Location
{
public:

    Location() = default;

    Location(const char *file, const char *func, int line)
      : file_(file),
        func_(func),
        line_(line)
    {}

    const char *file_ = __FILE__;
    const char *func_ = __FUNCTION__;
    int  line_        = __LINE__;
};


void print_location(const Location &location = Location());


#endif