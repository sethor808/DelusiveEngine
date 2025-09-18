#include "Component.h"
#include "DelusiveRegistry.h"
#include <imgui/imgui.h>
#include <sstream>

Component::Component()
    : registry(std::make_unique<PropertyRegistry>())
{
    
    bool initialized = false;
    if (!initialized) {
        RegisterProperties();
        initialized = true;
    }
}

void Component::RegisterProperties() {
	transform.RegisterProperties(*registry);
	registry->Register("name", &name);
	registry->Register("enabled", &enabled);
}

void Component::SetName(const std::string& newName) {
	name = newName;
}

void Component::Serialize(std::ostream& out) const {
	registry->Serialize(out);
}

void Component::Deserialize(std::istream& in) {
    std::stringstream buffer;

    std::string line;
    while (std::getline(in, line)) {
        if (line == "[/Component]") {
            break; // stop at end of component
        }
        buffer << line << "\n";  // collect block into buffer
    }

    // Now let registry parse key=value pairs
    std::istringstream block(buffer.str());
    registry->Deserialize(block);
}

void Component::DrawImGui() {
    ImGui::Text("%s", GetType());
	registry->DrawImGui();
}