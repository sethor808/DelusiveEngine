#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Core/DelusiveVar.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <limits>

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

void PropertyRegistry::Serialize(DelusiveParser::DataBlock& out) const {
    out.category = category;
    out.type = type;

    for (const auto& prop : properties) {
        std::ostringstream ss;
        ss << std::setprecision(std::numeric_limits<float>::max_digits10);
        prop->Serialize(ss);
        out.properties[prop->GetName()] = ss.str();
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

void PropertyRegistry::Resolve(DelusiveInstance& instance) {
    for (auto& prop : properties) {
        prop->Resolve(instance);
    }
}

void PropertyRegistry::Collect(std::vector<DelusiveParser::DataBlock>& out) const {
    for (const auto& prop : properties) {
        prop->Collect(out);
    }
}

void PropertyRegistry::DrawImGui() {
    for (auto& prop : properties) {
        prop->DrawImGui();
    }
}