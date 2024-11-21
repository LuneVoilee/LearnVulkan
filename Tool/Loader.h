#pragma once
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class Loader
{
public:
    static std::vector<char> ReadFile(const std::string& filename);
};
