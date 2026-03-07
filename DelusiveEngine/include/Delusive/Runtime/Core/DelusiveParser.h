#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map> // Add this include

namespace DelusiveParser {
    struct DataBlock {
        std::string category;
        std::string type;

        std::unordered_map<std::string, std::string> properties;

        std::vector<DataBlock> children;
    };

    std::vector<DataBlock> ParseFile(std::istream&);
    bool ReadDataBlock(std::istream&, DataBlock&);
}