#include "utils.h"

#include <filesystem>
#include <iostream>
#include <fstream>

bool tryWriteAll(const std::string& path, const std::vector<std::uint8_t>& data)
{
    if (std::filesystem::exists(path) && !std::filesystem::remove(path)) {
        return false;
    }

    try {
        std::fstream writer(path, std::ios::out | std::ios::binary);

        if (!writer.is_open()) {
            return false;
        }

        writer.seekg(0, std::ios::beg);

        writer.write((const char*)data.data(), data.size());

        writer.close();
    }
    catch (...) {
        return false;
    }

    return true;
}

bool tryReadAll(const std::string& path, std::vector<std::uint8_t>& data, std::size_t sizeMax)
{
    try {
        std::fstream reader(path, std::ios::in | std::ios::binary);

        if (!reader.is_open()) {
            return false;
        }

        reader.seekg(0, std::ios::end);

        std::size_t size = (std::size_t)reader.tellg();
        if (sizeMax < size) {
            reader.close();
            return false;
        }

        reader.seekg(0, std::ios::beg);

        data.resize(size);
        reader.read((char*)data.data(), data.size());

        reader.close();
    }
    catch (...) {
        return false;
    }

    return true;
}
