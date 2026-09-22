#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <sstream>
#include <iostream>
#include <algorithm>

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
    std::string line;

    while (std::getline(in, line)) {
        line = Trim(line);
        if (line.empty() || line.front() == '#') continue;

        if (line.front() == '[') {
            blocks.push_back(ParseHeader(line));
            continue;
        }

        if (blocks.empty()) continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        blocks.back().properties[Trim(line.substr(0, eq))] = Trim(line.substr(eq + 1));
    }

    return blocks;
}

void DelusiveParser::WriteBlock(std::ostream& out, const DataBlock& block) {
    out << "[" << block.category;
    if (!block.type.empty()) out << " " << block.type;
    if (block.id.IsValid())  out << " " << block.id.ToString();
    out << "]\n";

    //properties is unordered, so sort the keys to keep saves diff stable
    std::vector<const std::string*> keys;
    keys.reserve(block.properties.size());

    for (const auto& [key, value] : block.properties) {
        keys.push_back(&key);
    }

    std::sort(keys.begin(), keys.end(),
        [](const std::string* a, const std::string* b) { return *a < *b; });

    for (const std::string* key : keys) {
        out << *key << "=" << block.properties.at(*key) << "\n";
    }
}
