#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <sstream>
#include <iostream>

// Registry Definitions
void PropertyRegistry::Serialize(std::ostream& out) const {
    for (const auto& prop : properties) {
        out << prop->GetName() << "=";
        prop->Serialize(out);
        out << "\n";
    }
}

void PropertyRegistry::Deserialize(std::istream& in) {
    std::streampos lastPos;
    std::string line;

    while (true) {
        lastPos = in.tellg(); // remember where we were
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;

        if (line[0] == '[') {
            // Rewind so parent can see this header line
            in.seekg(lastPos);
            break;
        }

        // Ignore stray closing braces
        if (line == "}") {
            break;
        }

        // Handle block start
        if (line == "{") {
            // This should never appear alone at top level
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // trim
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));

        for (auto& prop : properties) {
            if (prop->GetName() == key) {

                std::istringstream iss(value);
                prop->Deserialize(iss);

                // Check for block
                std::streampos blockStart = in.tellg();
                std::string nextLine;

                nextLine.erase(0, nextLine.find_first_not_of(" \t\r\n"));
                nextLine.erase(nextLine.find_last_not_of(" \t\r\n") + 1);
                if (std::getline(in, nextLine)) {
                    if (nextLine == "{") {

                        std::stringstream buffer;
                        int depth = 1;

                        while (std::getline(in, nextLine)) {
                            if (nextLine == "{") depth++;
                            else if (nextLine == "}") {
                                depth--;
                                if (depth == 0) break;
                            }

                            buffer << nextLine << "\n";
                        }

                        std::istringstream blockStream(buffer.str());
                        prop->Deserialize(blockStream);
                    }
                    else {
                        in.seekg(blockStart);
                    }
                }

                break;
            }
        }
    }
}

void PropertyRegistry::Deserialize(const DelusiveParser::DataBlock& block) {
    for (auto& prop : properties)
    {
        auto it = block.properties.find(prop->GetName());

        if (it == block.properties.end())
            continue;

        std::istringstream ss(it->second);
        prop->Deserialize(ss);
    }
}

void PropertyRegistry::DrawImGui() {
    for (auto& prop : properties) {
        prop->DrawImGui();
    }
}