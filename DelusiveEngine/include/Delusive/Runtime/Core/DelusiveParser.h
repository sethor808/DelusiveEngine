#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map> // Add this include
#include <Delusive/Runtime/Utils/UUID.h>

namespace DelusiveParser {
    struct DataBlock {
        std::string category;
        std::string type;
        UUID id;

        std::unordered_map<std::string, std::string> properties;
<<<<<<< Updated upstream

        std::vector<DataBlock> children;
=======
>>>>>>> Stashed changes
    };

    DataBlock ParseHeader(const std::string&);
    std::vector<DataBlock> ParseFile(std::istream&);
<<<<<<< Updated upstream
    bool ReadDataBlock(std::istream&, DataBlock&);
=======

    //Helpers
    std::string Trim(const std::string&);
>>>>>>> Stashed changes
}