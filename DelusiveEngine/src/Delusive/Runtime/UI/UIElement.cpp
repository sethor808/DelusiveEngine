#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <algorithm>
#include <iostream>
#include <sstream>

UIElement::UIElement(DelusiveInstance& instance)
    : instance(instance), registry(std::make_unique<PropertyRegistry>()), id(UUID::GenerateRandom())
{
	//Derived constructors call RegisterProperties - calling it here would
	//dispatch to the base GetType(), which is pure virtual during base construction
}

UIElement::~UIElement() {
    if(parentCanvas) {
        parentCanvas->idManager.Unregister(id);
	}
}

void UIElement::LinkCanvas(UICanvas* canvas) {
    parentCanvas = canvas;
    for (auto& child : children) {
        if (child) {
            child->LinkCanvas(canvas);
        }
	}

    //Just for safe measures - probably extra code
	if (parentCanvas == nullptr) return;

	//Generate ID here since we may not be linked upwards until after construction
	parentCanvas->idManager.Register(this, id);
}

void UIElement::RegisterProperties(){
    registry->category = "UIElement";
    registry->type = GetType();

	registry->Register("id", &id);
	registry->Register("name", &name);
	registry->Register("enabled", &enabled);
	registry->Register("position", &position);
	registry->Register("size", &size);
	registry->Register("bindEnabled", &bindEnabled);
	registry->Hide("bindEnabled"); //Drawn in the Data binding section
}

void UIElement::Move(const glm::vec2& delta) {
	position += delta;
	for (auto& child : children) {
		child->Move(delta);
	}
}

void UIElement::ApplyBinding(const UIBindItem& item) {
	if (const std::string* value = DelusiveUI::FetchBindField(item, bindEnabled)) {
		enabled = *value != "0";
	}

	for (auto& child : children) {
		child->ApplyBinding(item);
	}
}

bool UIElement::RemoveChild(UIElement* target) {
	auto found = std::find_if(children.begin(), children.end(),
		[target](const std::unique_ptr<UIElement>& child) { return child.get() == target; });
	if (found == children.end()) return false;

	children.erase(found);
	return true;
}

std::vector<UIElement*> UIElement::GetChildren() {
    std::vector<UIElement*> refs;
    refs.reserve(children.size());
    
    for (auto& c : children) {
        refs.push_back(c.get());
    }

    return refs;
}

void UIElement::DrawImGui() {
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		UUID payload = id;

		ImGui::SetDragDropPayload(
			"UI_ELEMENT_UUID",
			&payload,
			sizeof(UUID)
		);

		ImGui::Text("UI Element");
		ImGui::Text("Type: %s", GetType().c_str());
		ImGui::Text("ID: %s", id.ToString().c_str());

		ImGui::EndDragDropSource();
	}

	registry->DrawImGui();
	DrawBindingImGui();
	DrawChildrenImGui();
}

void UIElement::DrawBindField(const char* label, std::string& field, const char* help) {
	ImGui::PushID(label);
	if (ImGui::BeginCombo(label, field.empty() ? "<None>" : field.c_str())) {
		if (ImGui::Selectable("<None>", field.empty())) field.clear();

		for (const UIBindField& option : DelusiveUI::ListBindFields()) {
			if (ImGui::Selectable(option.name, field == option.name)) field = option.name;
			ImGui::SameLine();
			ImGui::TextDisabled("%s", option.help);
		}
		ImGui::EndCombo();
	}
	ImGui::SetItemTooltip("%s", help);
	ImGui::PopID();
}

void UIElement::DrawBindingImGui() {
	//Only means something inside a repeat container that has a data source
	ImGui::SeparatorText("Data binding");
	DrawBindField("Show only when", bindEnabled, "Shown while this field is not 0. Leave empty to always show.");
}

void UIElement::DrawChildrenImGui() {
	ImGui::SeparatorText("Children");

	//Removal waits until the loop is done so the list stays valid
	UIElement* toRemove = nullptr;

	for (auto& child : children) {
		if (!child) continue;

		ImGui::PushID(child.get());
		const std::string label = "[" + child->GetType() + "] " + child->GetName();
		const bool open = ImGui::TreeNodeEx("##child", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap, "%s", label.c_str());

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40.0f);
		if (ImGui::SmallButton("Remove")) toRemove = child.get();

		if (open) {
			child->DrawImGui();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if (toRemove) RemoveChild(toRemove);

	if (ImGui::Button("+ Add Child Element", ImVec2(-FLT_MIN, 0))) {
		ImGui::OpenPopup("AddUIElementPopup");
	}

	if (ImGui::BeginPopup("AddUIElementPopup")) {
		std::string type = DelusiveUI::DrawUIElementAddMenu();
		if (!type.empty()) {
			auto newElement = DelusiveUI::CreateUIElementByType(type, instance);
			if (newElement) AddChild(std::move(newElement));
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void UIElement::Deserialize(DelusiveParser::DataBlock& in) {
	registry->Deserialize(in);
	//Owned objects pull their recipes from the library, which is already populated
	registry->Resolve(instance);

	if (!SavesChildren()) return;

	children.clear();

	auto list = in.properties.find("children");
	if (list == in.properties.end()) return;

	std::istringstream ids(list->second);
	std::string idText;

	while (ids >> idText) {
		UUID childID;
		childID.FromString(idText);

		const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(childID);
		if (!recipe) {
			std::cerr << "[UIElement] Missing child recipe " << idText << " for " << name << std::endl;
			continue;
		}

		std::unique_ptr<UIElement> child = DelusiveBuild<UIElement>(*recipe, instance);
		if (!child) continue;

		child->LinkCanvas(parentCanvas);
		children.push_back(std::move(child));
	}
}

void UIElement::Serialize(DelusiveParser::DataBlock& out) const {
	registry->Serialize(out);

	if (!SavesChildren()) return;

	//Ownership is a UUID list - each child writes its own block
	std::ostringstream ids;
	for (const auto& child : children) {
		if (child) ids << child->GetID().ToString() << " ";
	}
	out.properties["children"] = ids.str();
}

std::unique_ptr<UIElement> UIElement::Clone() const {
	return DelusiveClone<UIElement>(*this, instance);
}

void UIElement::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
	DelusiveParser::DataBlock self;
	Serialize(self);
	self.id = id;
	out.push_back(std::move(self));

	CollectOwned(out);

	if (!SavesChildren()) return;

	for (const auto& child : children) {
		if (child) child->CollectBlocks(out);
	}
}

void UIElement::CollectOwned(std::vector<DelusiveParser::DataBlock>& out) const {
	registry->Collect(out);
}
