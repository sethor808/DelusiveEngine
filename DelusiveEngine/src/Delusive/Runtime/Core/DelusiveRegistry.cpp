#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Core/DelusiveVar.h>
#include <sstream>
#include <iostream>

// Correct template constructor definition
template<typename T>
DelusiveVar<T>::DelusiveVar(PropertyRegistry& registry, const char* name, T* value) {
    registry.Register<T>(name, value);
}

// Registry Definitions
void PropertyRegistry::Serialize(std::ostream& out) const {
    if (category.empty()) {
        std::cerr << "Serialize called with empty category string!" << std::endl;
    }
    out << "[" << category;

    if (type.empty()) {
        out << "]\n";
    }
    else {
        out << " " << type << "]\n";
    }

    for (const auto& prop : properties) {
        out << prop->GetName() << "=";
        prop->Serialize(out);
        out << "\n";
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