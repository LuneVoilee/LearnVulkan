#include "Loader.h"

std::vector<char> Loader::ReadFile(const std::string& filename)
{
    namespace fs = std::filesystem;
    if (!fs::exists(filename))
    {
        throw std::runtime_error("File does not exist: " + filename);
    }
    size_t fileSize = fs::file_size(filename);

    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);

    return buffer;
}
