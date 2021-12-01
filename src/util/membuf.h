#pragma once
#include <iostream>

// cant find exact source but here is a similar implementation
// https://stackoverflow.com/a/13059195/10945691
// this lets you stream into a char array
// (trying to avoid copying the char array with either .str() calls to a stringstream or similar streams)
struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};
