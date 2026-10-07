#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Components/DelusiveComponents.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <limits>
#include <sstream>

Agent::Agent(DelusiveInstance& instance)
	: instance(instance), registry(std::make_unique<PropertyRegistry>())
{
	//Derived constructors call RegisterProperties - calling it here would
	//dispatch to the base GetType(), which is pure virtual during base construction
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

void Agent::MoveComponent(Component* target, int delta) {
	auto found = std::find_if(components.begin(), components.end(),
		[target](const std::unique_ptr<Component>& c) { return c.get() == target; });
	if (found == components.end()) return;

	const int from = static_cast<int>(found - components.begin());
	const int to = std::clamp(from + delta, 0, static_cast<int>(components.size()) - 1);
	if (from == to) return;

	std::unique_ptr<Component> moving = std::move(*found);
	components.erase(found);
	components.insert(components.begin() + to, std::move(moving));
}

void Agent::DrawPropertiesImGui() {
	registry->DrawImGui();
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
	if (!target) return;
	//The lookup must not outlive the component
	componentLookup.erase(target->GetID());
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

#pragma region Block serialization

void Agent::Serialize(DelusiveParser::DataBlock& out) const {
	registry->Serialize(out);

	//Ownership is a UUID list - each component writes its own block
	std::ostringstream ids;
	for (const auto& comp : components) {
		if (comp) ids << comp->GetID().ToString() << " ";
	}
	out.properties["components"] = ids.str();
}

void Agent::Deserialize(DelusiveParser::DataBlock& in) {
	registry->Deserialize(in);
	//Owned objects pull their recipes from the library, which is already populated
	registry->Resolve(instance);

	components.clear();
	componentLookup.clear();

	auto list = in.properties.find("components");
	if (list == in.properties.end()) return;

	std::istringstream ids(list->second);
	std::string idText;

	while (ids >> idText) {
		UUID componentID;
		componentID.FromString(idText);

		const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(componentID);
		if (!recipe) {
			std::cerr << "[Agent] Missing component recipe " << idText
				<< " for " << name << std::endl;
			continue;
		}

		//Owner first - some components reach their agent while loading
		std::unique_ptr<Component> comp = DelusiveBuild<Component>(*recipe, instance,
			[this](Component& c) { c.SetOwner(this); });
		if (!comp) continue;

		AddRawComponent(std::move(comp));
	}
}

std::unique_ptr<Agent> Agent::FromRecipe(const DelusiveParser::DataBlock& recipe, DelusiveInstance& instance, Scene* scene) {
	//Scene first - scripts look up other agents while their components load
	return DelusiveBuild<Agent>(recipe, instance, [scene](Agent& agent) {
		if (scene) agent.LinkScene(scene);
	});
}

std::unique_ptr<Agent> Agent::Clone(Scene* scene) const {
	return DelusiveClone<Agent>(*this, instance, [scene](Agent& agent) {
		if (scene) agent.LinkScene(scene);
	});
}

std::unique_ptr<Agent> Agent::LoadFromFile(const std::string& path, DelusiveInstance& instance, Scene* scene) {
	if (!instance.delusiveLibrary.LoadFile(path)) return nullptr;

	//The first Agent block is the file's root - the rest are its components
	for (const DelusiveParser::DataBlock* block : instance.delusiveLibrary.ListFile(path)) {
		if (block->category == "Agent") {
			return FromRecipe(*block, instance, scene);
		}
	}

	std::cerr << "[Agent] No Agent block in " << path << std::endl;
	return nullptr;
}

bool Agent::SaveToFile(const std::string& path, bool* remapped) {
	if (remapped) *remapped = false;
	if (!id.IsValid()) id = UUID::GenerateRandom();

	std::vector<DelusiveParser::DataBlock> blocks;
	CollectBlocks(blocks);

	DelusiveLibrary::IDRemap remap;
	if (!instance.delusiveLibrary.WriteFile(path, std::move(blocks), &remap)) return false;

	if (remapped) *remapped = !remap.empty();
	return true;
}

void Agent::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
	DelusiveParser::DataBlock self;
	Serialize(self);
	self.id = id;
	out.push_back(std::move(self));

	for (const auto& comp : components) {
		if (comp) comp->CollectBlocks(out);
	}
}

#pragma endregion
