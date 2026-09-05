#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <sstream>
#include <iostream>

<<<<<<< Updated upstream
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
=======
std::string DelusiveParser::Trim(const std::string& s)
{
    const char* ws = " \t\r\n";

    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos)
        return "";

    size_t end = s.find_last_not_of(ws);

    return s.substr(start, end - start + 1);
}

DelusiveParser::DataBlock DelusiveParser::ParseHeader(const std::string& line) {
    DelusiveParser::DataBlock block;

    size_t close = line.find_last_of(']');

    std::string inner = (close == std::string::npos)
        ? line.substr(1)
        : line.substr(1, close - 1);

    std::istringstream header(inner);
    std::string idText;
    header >> block.category >> block.type >> idText;
    if (!idText.empty()) block.id.FromString(idText);

    return block;
}

std::vector<DelusiveParser::DataBlock> DelusiveParser::ParseFile(std::istream& in) {
    std::vector<DataBlock> blocks;
>>>>>>> Stashed changes
    std::string line;

    while (std::getline(in, line)) {
        line = Trim(line);
        if (line.empty() || line.front() == '#') continue;

<<<<<<< Updated upstream
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
=======
        if (line.front() == '[') {
            blocks.push_back(ParseHeader(line));
            continue;
>>>>>>> Stashed changes
        }

        if (blocks.empty()) continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        blocks.back().properties[Trim(line.substr(0, eq))] = Trim(line.substr(eq + 1));
    }

    return blocks;
}

