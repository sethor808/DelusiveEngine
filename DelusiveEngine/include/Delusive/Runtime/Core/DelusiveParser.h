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
        std::unordered_map<std::string, DataBlock> registryProperty;
    };

    std::vector<DataBlock> ParseFile(std::istream&);
    bool ReadDataBlock(std::istream&, DataBlock&);

    //Helpers
    std::string Trim(const std::string&);
    bool PeekNextIsBlock(std::istream& in);
}