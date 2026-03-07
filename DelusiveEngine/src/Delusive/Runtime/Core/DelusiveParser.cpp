#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <sstream>
#include <iostream>

std::vector<DelusiveParser::DataBlock> DelusiveParser::ParseFile(std::istream& in) {
    std::vector<DelusiveParser::DataBlock> blocks;

    while (true) {
        DelusiveParser::DataBlock block;

        if (!DelusiveParser::ReadDataBlock(in, block)) break;

        blocks.push_back(std::move(block));
    }

    return blocks;
}

bool DelusiveParser::ReadDataBlock(std::istream& in, DataBlock& block) {
    std::string line;

    //Search for header
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line.front() == '[' && line[1] != '/') break;
    }

    if (!in) return false;

    //Remove brackets
    line = line.substr(1, line.size() - 2);
    std::istringstream header(line);

    header >> block.category;
    header >> block.type;

    std::string endTag = "[/" + block.category + "]";

    while (true) {
        std::streampos pos = in.tellg();

        if (!std::getline(in, line)) break;
        if (line.empty()) continue;
        if (line == endTag) break;

        if (line.front() == '[' && line[1] != '/')
        {
            in.seekg(pos);

            DataBlock child;
            if (ReadDataBlock(in, child))
                block.children.push_back(std::move(child));

            continue;
        }

        auto eq = line.find('=');
        if (eq != std::string::npos)
        {
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            block.properties[key] = value;
        }
    }

    return true;
}