#include "Component.h"
#include <sstream>

Component::Component() {
	RegisterProperties();
}

void Component::RegisterProperties() {
	transform.RegisterProperties(registry);
	registry.Register("name", &name);
	registry.Register("enabled", &enabled);
	registry.Register("texturePath", &texturePath);
}

void Component::SetName(const std::string& newName) {
	name = newName;
}

void Component::Serialize(std::ofstream& out) const {
	registry.Serialize(out);
}

void Component::Deserialize(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line == "[/Component]") {
            break; // end of this component block
        }

        std::istringstream iss(line);
        registry.Deserialize(iss);

        /*
        // Parse as key + value(s)
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key.empty()) continue;

        for (auto& prop : registry.properties) {
            if (prop->name == key) {
                prop->Deserialize(iss);
                break;
            }
        }
        */
    }
}

void Component::DrawImGui() {
	registry.DrawImGui();
}