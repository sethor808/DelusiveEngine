#include <DelusiveInternal/Core/DelusiveRegistry.h>
#include <sstream>
#include <iostream>

// ----------------------
// Registry Definitions
// ----------------------
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
                break;
            }
        }
    }
}

void PropertyRegistry::DrawImGui() {
    for (auto& prop : properties) {
        prop->DrawImGui();
    }
}