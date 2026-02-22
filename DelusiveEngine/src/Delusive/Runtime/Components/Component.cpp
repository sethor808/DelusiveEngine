#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <sstream>

Component::Component(DelusiveRenderer& renderer)
    :   renderer(renderer),
        registry(std::make_unique<PropertyRegistry>()), transform(std::make_unique<TransformComponent>())
{
    
    bool initialized = false;
    if (!initialized) {
        RegisterProperties();
        initialized = true;
    }
}

Component::~Component() = default;

void Component::RegisterProperties() {
    registry->Register("id", &id);
	transform->RegisterProperties(*registry);
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