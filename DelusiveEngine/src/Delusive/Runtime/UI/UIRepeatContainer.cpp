#define DELUSIVE_EDITOR_MODE
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>

UIRepeatContainer::UIRepeatContainer(DelusiveInstance& instance)
	: UIElement(instance)
{
	name = "New UIRepeatContainer";
	RegisterProperties();
}

void UIRepeatContainer::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("count", &count);
	registry->Register("rows", &rows);
	registry->Register("spacing", &spacing);
	registry->Register("prototype", &prototype);
	registry->Hide("prototype"); //Edited in the Prototype section below
	registry->Register("source", &source);
	registry->Hide("source");
	registry->Register("bindCount", &bindCount);
	registry->Hide("bindCount"); //Drawn in the Data binding section
}

void UIRepeatContainer::Update(float deltaTime) {
	if (!source.empty() && parentCanvas) {
		std::vector<UIBindItem> items = DelusiveUI::FetchBindSource(source, *parentCanvas);

		//Only rebuild when the data changed, e.g. a talisman took a hit
		if (items != boundItems) {
			boundItems = std::move(items);
			Rebuild();
		}
	}

	//Dragged by the inspector rather than moved by code
	if (position != placedAt) {
		const glm::vec2 delta = position - placedAt;
		placedAt = position;
		for (auto& child : children) {
			child->Move(delta);
		}
	}

	UIElement::Update(deltaTime);
}

void UIRepeatContainer::Move(const glm::vec2& delta) {
	UIElement::Move(delta);
	placedAt = position;
}

void UIRepeatContainer::Rebuild() {
	if (!source.empty()) SetCount(static_cast<int>(boundItems.size()));
	RegenerateChildren();

	for (size_t i = 0; i < children.size() && i < boundItems.size(); ++i) {
		children[i]->ApplyBinding(boundItems[i]);
	}
}

void UIRepeatContainer::NotifyEdited() {
	//Copies inside a bound item are rebuilt by whatever rebuilds that item
	if (!hasParentItem) Rebuild();
}

void UIRepeatContainer::ApplyBinding(const UIBindItem& item) {
	if (const std::string* value = DelusiveUI::FetchBindField(item, bindCount)) {
		hasParentItem = true;
		SetCount(std::atoi(value->c_str()));
		RegenerateChildren();
	}

	UIElement::ApplyBinding(item);
}

void UIRepeatContainer::Draw(const glm::mat4& projection) {
	if (!enabled) return;
	UIElement::Draw(projection);

#ifdef DELUSIVE_EDITOR_MODE
	//Shows where copies will go while there are none
	if (prototype && children.empty() && source.empty() && bindCount.empty()) {
		instance.renderer.PushAlpha(0.3f);
		prototype->Draw(projection);
		instance.renderer.PopAlpha();
	}
#endif
}

std::vector<UIElement*> UIRepeatContainer::GetEditorChildren() {
	return prototype ? std::vector<UIElement*>{ prototype.get() } : std::vector<UIElement*>{};
}

void UIRepeatContainer::AddEditorChild(std::unique_ptr<UIElement> child) {
	child->LinkCanvas(parentCanvas);
	prototype = std::move(child);
	Rebuild();
}

bool UIRepeatContainer::RemoveEditorChild(UIElement* child) {
	if (!prototype || prototype.get() != child) return false;

	prototype = nullptr;
	children.clear();
	return true;
}

void UIRepeatContainer::DrawBindingImGui() {
	UIElement::DrawBindingImGui();
	DrawBindField("Count from", bindCount, "Repeats this many times, read from the field of the item this container sits in (for example hp, to make one string per HP).");

	if (ImGui::BeginCombo("Repeat over", source.empty() ? "<None>" : source.c_str())) {
		if (ImGui::Selectable("<None>", source.empty())) source.clear();
		for (const std::string& option : DelusiveUI::ListBindSources()) {
			if (ImGui::Selectable(option.c_str(), option == source)) source = option;
		}
		ImGui::EndCombo();
	}
	ImGui::SetItemTooltip("Game data to repeat the prototype over, one copy per entry (PlayerTalismans: one per unbroken talisman). Leave empty to use Count.");
}

void UIRepeatContainer::DrawChildrenImGui() {
	ImGui::SeparatorText("Prototype");

	if (!prototype) {
		ImGui::TextDisabled("Every copy is built from the prototype.");
		if (ImGui::Button("Create Prototype", ImVec2(-FLT_MIN, 0))) {
			ImGui::OpenPopup("AddPrototypePopup");
		}

		if (ImGui::BeginPopup("AddPrototypePopup")) {
			std::string type = DelusiveUI::DrawUIElementAddMenu();
			if (!type.empty()) {
				auto newPrototype = DelusiveUI::CreateUIElementByType(type, instance);
				if (newPrototype) AddEditorChild(std::move(newPrototype));
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return;
	}

	ImGui::TextWrapped("Edit the prototype under this element in the hierarchy. The %d copies update when you do.", static_cast<int>(children.size()));
}

void UIRepeatContainer::SetPrototype(std::unique_ptr<UIElement> element) {
	prototype = std::move(element);
}

void UIRepeatContainer::RegenerateChildren() {
	children.clear();
	placedAt = position;
	if (!prototype) return;
	if (count <= 0) return;

	// Basic layout: if rows <= 1 -> single row
	int rowsToUse = (rows <= 1) ? 1 : rows;
	int cols = (rowsToUse == 1) ? count : ((count + rowsToUse - 1) / rowsToUse);

	// Use spacing.x for horizontal, spacing.y for vertical
	for (int idx = 0; idx < count; ++idx) {
		//Each item is its own element, so it needs its own ids
		auto item = DelusiveInstantiate<UIElement>(*prototype.get(), instance);
		if (!item) continue;

		int r = (rowsToUse == 1) ? 0 : (idx / cols);   // row index
		int c = (rowsToUse == 1) ? idx : (idx % cols); // column index

		glm::vec2 offset = glm::vec2(spacing * float(c), spacing * float(r)); //TODO: change spacing to a vec2 for x and y spacing
		item->Move(position + offset - item->GetPosition());

		// AddChild should take ownership and set parent properly
		AddChild(std::move(item));
	}
}