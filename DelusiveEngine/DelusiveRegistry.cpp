#include "DelusiveRegistry.h"
#include <sstream>
#include <iostream>

// ----------------------
// Registry Definitions
// ----------------------
void PropertyRegistry::Serialize(std::ostream& out) const {
    for (const auto& prop : properties) {
        out << prop->GetName() << " ";
        prop->Serialize(out);
        out << "\n";
    }
}

void PropertyRegistry::Deserialize(std::istream& in) {
    std::string propName;
    while (in >> propName) {
        for (auto& prop : properties) {
            if (prop->GetName() == propName) {
                prop->Deserialize(in);
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