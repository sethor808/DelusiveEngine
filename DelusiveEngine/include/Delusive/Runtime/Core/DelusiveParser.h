#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map> // Add this include
#include <Delusive/Runtime/Core/UUID.h>

namespace DelusiveParser {
    struct DataBlock {
        std::string category;
        std::string type;
        UUID id;

        std::unordered_map<std::string, std::string> properties;
    };

    DataBlock ParseHeader(const std::string&);
    std::vector<DataBlock> ParseFile(std::istream&);

    //Counterpart to ParseFile - keeps read and write formats in one place
    void WriteBlock(std::ostream&, const DataBlock&);

    //Helpers
    std::string Trim(const std::string&);
}