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

std::string Trim(const std::string& s)
{
    const char* ws = " \t\r\n";

    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos)
        return "";

    size_t end = s.find_last_not_of(ws);

    return s.substr(start, end - start + 1);
}

bool DelusiveParser::PeekNextIsBlock(std::istream& in) {
    std::streampos pos = in.tellg();

    std::string line;

    while (std::getline(in, line))
    {
        line = Trim(line);

        if (line.empty())
            continue;

        in.seekg(pos);
        return line.front() == '[';
    }

    in.seekg(pos);
    return false;
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

        auto eq = line.find('=');
        if (eq != std::string::npos)
        {
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            if (value.empty() && PeekNextIsBlock(in))
            {
                DataBlock child;

                if (ReadDataBlock(in, child))
                {
                    block.registryProperty[key] = std::move(child);
                }
                continue;
            }

            block.properties[key] = value;
        }
    }

    return true;
}