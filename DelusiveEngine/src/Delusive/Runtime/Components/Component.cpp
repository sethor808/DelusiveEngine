#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <sstream>

Component::Component(DelusiveInstance& instance)
    :   instance(instance),
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
    registry->category = "Component";
    registry->type = GetType();

    registry->Register("id", &id);
	transform->RegisterProperties(*registry);
	registry->Register("name", &name);
	registry->Register("enabled", &enabled);
}

void Component::SetName(const std::string& newName) {
	name = newName;
}

void Component::DrawImGui() {
    ImGui::Text("%s", GetType());
	registry->DrawImGui();
}
void Component::Deserialize(DelusiveParser::DataBlock& in) {
	registry->Deserialize(in);
	//Owned objects pull their recipes from the library, which is already populated
	registry->Resolve(instance);
}

void Component::Serialize(DelusiveParser::DataBlock& out) const {
	registry->Serialize(out);
}

void Component::CollectOwned(std::vector<DelusiveParser::DataBlock>& out) const {
	registry->Collect(out);
}
