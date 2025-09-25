#include "DelusiveRegistry.h"
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
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') break; // section marker, stop here

        auto pos = line.find('=');
        if (pos == std::string::npos) continue; // malformed

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // trim
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));

        // find matching property
        for (auto& prop : properties) {
            if (prop->GetName() == key) {
                std::istringstream iss(value);
                prop->Deserialize(iss);  // only parse RHS
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