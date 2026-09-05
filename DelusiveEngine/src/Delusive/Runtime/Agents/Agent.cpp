#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Components/DelusiveComponents.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <limits>
#include <sstream>

Agent::Agent(DelusiveInstance& instance)
	: instance(instance), registry(std::make_unique<PropertyRegistry>())
{
	RegisterProperties();
}

Agent::~Agent() {

}

void Agent::RegisterProperties() {
    registry->category = "Agent";
    registry->type = this->GetType();
    registry->Register("id", &id);
	registry->Register("name", &name);
	transform.RegisterProperties(*registry);
}

void Agent::Update(float deltaTime) {
    for (auto& c : components) {
        c->Update(deltaTime);
    }
}

void Agent::Draw(const glm::mat4& projection) const{
    for (auto& c : components) {
        c->Draw(projection);
    }
}

void Agent::SetEditorMode(bool selected) {
	editorMode = selected;
	if (selected) {}
}

//TODO: Rip out interaction handling and move almost all of this into the editor itself
void Agent::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
	if (editorMode) {
		if (editorMode) {
			glm::vec2 center = transform.position;
			glm::vec2 halfSize = transform.scale * 0.5f;

			glm::vec2 min = center - halfSize;
			glm::vec2 max = center + halfSize;

			bool mouseOver = worldMouse.x >= min.x && worldMouse.x <= max.x &&
				worldMouse.y >= min.y && worldMouse.y <= max.y;

			if (!mouseDown && interaction.currentAction == EditorAction::None) {
				interaction.isSelected = mouseOver;
			}

			if (mouseDown && interaction.currentAction == EditorAction::None && mouseOver) {
				interaction.currentAction = EditorAction::Drag;
				interaction.dragOffset = (worldMouse - center) / transform.scale;
			}

			if (!mouseDown) {
				interaction.currentAction = EditorAction::None;
			}

			if (interaction.currentAction == EditorAction::Drag) {
				glm::vec2 delta = (worldMouse) - (interaction.dragOffset * transform.scale);
				transform.position = delta;
			}
		}
	}
	else {
		for (auto& c : components) {
			c->HandleMouse(worldMouse, mouseDown);
		}	
	}
}

void Agent::SetPosition(const glm::vec2& pos) {
	transform.position = pos;
}

void Agent::SetRotation(const float rotation) {
	transform.rotation = rotation;
}

void Agent::SetScale(const glm::vec2& scale) {
	transform.scale = scale;
}

void Agent::SetTransform(TransformComponent& newTransform) {
    transform = newTransform;
}

void Agent::AddRawComponent(std::unique_ptr<Component> component) {
	component->SetOwner(this);

    if (!component->GetID().IsValid()) {
        component->SetID(UUID::GenerateRandom());
    }
    componentLookup[component->GetID()] = component.get();

	components.push_back(std::move(component));
}

Component* Agent::GetComponentByID(UUID targetID)
{
    auto it = componentLookup.find(targetID);
    if (it != componentLookup.end()) {
        return it->second;
    }
    return nullptr;
}

TransformComponent& Agent::GetTransform() {
	return transform;
}

TransformComponent& Agent::GetTransform() const {
    return const_cast<TransformComponent&>(transform);
}

void Agent::DrawImGui() {
	registry->DrawImGui();
	ImGui::Separator();

	int componentID = 0;
	for (const auto& comp : components) {
		ImGui::PushID(componentID++);
		ImGui::NewLine();
		ImGui::Separator();
		comp->DrawImGui();
		ImGui::PopID();
	}

    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        std::string type = DelusiveComponentFactory::DrawComponentAddMenu();

        if (!type.empty())
        {
            ScriptManager& scriptManager = sceneLink->GetScriptManager();

            auto comp = DelusiveComponentFactory::CreateComponentByType(type, instance);

            if (comp)
                AddRawComponent(std::move(comp));

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Agent::RemoveComponentByPointer(Component* target) {
	components.erase(
		std::remove_if(
			components.begin(),
			components.end(),
			[target](const std::unique_ptr<Component>& c) {
				return c.get() == target;
			}
		),
		components.end()
	);
}

const std::vector<std::unique_ptr<Component>>& Agent::GetComponents() const {
	return components;
}

void Agent::CloneBaseProperties(Agent* copy, Scene* scene) const{
	copy->SetPosition(GetTransform().position);
	copy->SetRotation(GetTransform().rotation);
	copy->SetScale(GetTransform().scale);
	copy->SetName(GetName());
    copy->SetID(id);
	copy->LinkScene(scene);

	// Deep copy components
	for (const auto& comp : GetComponents()) {
		if (comp) {
			std::unique_ptr<Component> clone = comp->Clone();
			if (clone) copy->AddRawComponent(std::move(clone));
		}
	}
}