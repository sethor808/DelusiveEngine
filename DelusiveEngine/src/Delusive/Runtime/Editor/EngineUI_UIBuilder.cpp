#include <Delusive/Runtime/Editor/EngineUI.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Collision/HitData.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <imgui/imgui.h>
#include <functional>

void EngineUI::RenderUIBuilder(Scene& scene) {
	//Nothing updates the scene outside play, so the preview ticks here
	UIManager* ui = scene.GetSystem<UIManager>();
	if (ui) ui->Update(ImGui::GetIO().DeltaTime);

	if (!showUIBuilderPanel) return;
	ImGui::Begin(UIBuilderPanelWindow, &showUIBuilderPanel);

	//The preview player feeds bound elements, e.g. the talisman stack
	PlayerAgent* preview = scene.FetchPlayer();
	ImGui::BeginDisabled(!preview);
	if (ImGui::Button("Hit Preview")) preview->HitEvent(HitData{});
	ImGui::SameLine();
	if (ImGui::Button("Heal Preview")) {
		for (const auto& talisman : preview->GetTalismans()) talisman->Reset();
		preview->RecalculateStats();
	}
	ImGui::EndDisabled();
	ImGui::Separator();

	if (!editingCanvas) {
		ImGui::TextDisabled("Pick a canvas from the list at the top, or click New");
		ImGui::End();
		return;
	}

	//Hierarchy
	if (ImGui::BeginChild("UIHierarchy", ImVec2(ImGui::GetContentRegionAvail().x * 0.35f, 0), true)) {
		ImGui::Text("%s", editingCanvas->GetName().c_str());
		bool captures = editingCanvas->CapturesInput();
		if (ImGui::Checkbox("Captures input", &captures)) editingCanvas->SetCapturesInput(captures);
		ImGui::SetItemTooltip("Menus take clicks and keys from the game; leave off for HUDs");

		if (ImGui::Button("+ Add")) ImGui::OpenPopup("AddElement");
		ImGui::SetItemTooltip("Adds inside the selected element, or to the canvas when nothing is selected");
		if (ImGui::BeginPopup("AddElement")) {
			AddUIElement(DelusiveUI::DrawUIElementAddMenu(), selectedUIElement);
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupContextWindow("CanvasRightClick", ImGuiPopupFlags_MouseButtonRight)) {
			AddUIElement(DelusiveUI::DrawUIElementAddMenu(), nullptr);
			ImGui::EndPopup();
		}

		if (selectedUIElement && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && !ImGui::GetIO().WantTextInput
			&& ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
			uiDeleteTarget = selectedUIElement;
			uiDeleteParent = FindUIParent(selectedUIElement);
		}

		ImGui::Separator();
		for (UIElement* element : editingCanvas->GetElements()) {
			DrawUIElementNode(element, nullptr);
		}
		DeleteUIElement();
	}
	ImGui::EndChild();

	ImGui::SameLine();

	//Inspector (reuses the registered-property DrawImGui path)
	if (ImGui::BeginChild("UIInspector", ImVec2(0, 0), true)) {
		if (selectedUIElement) {
			selectedUIElement->DrawImGui();
		}
		else {
			ImGui::TextDisabled("Nothing selected.");
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void EngineUI::DrawUIElementNode(UIElement* element, UIElement* parent) {
	if (!element) return;

	//Runtime-built children (repeat copies, script output) stay out of the tree
	std::vector<UIElement*> children = element->GetChildren();
	children.resize(element->SavedChildCount());

	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_DefaultOpen |
		(children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) |
		(selectedUIElement == element ? ImGuiTreeNodeFlags_Selected : 0);

	std::string label = element->GetName().empty() ? element->GetType() : element->GetName();

	ImGui::PushID(element);
	bool open = ImGui::TreeNodeEx(label.c_str(), flags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		selectedUIElement = element;
	}

	if (ImGui::BeginPopupContextItem("ElementRightClick", ImGuiPopupFlags_MouseButtonRight)) {
		AddUIElement(DelusiveUI::DrawUIElementAddMenu(), element);
		ImGui::Separator();

		if (ImGui::MenuItem("Delete", "Del")) {
			uiDeleteTarget = element;
			uiDeleteParent = parent;
		}
		ImGui::EndPopup();
	}

	if (open) {
		for (UIElement* child : children) {
			DrawUIElementNode(child, element);
		}
		ImGui::TreePop();
	}

	ImGui::PopID();
}

void EngineUI::AddUIElement(const std::string& type, UIElement* parent) {
	if (type.empty()) return;
	auto element = DelusiveUI::CreateUIElementByType(type, instance);
	if (!element) return;

	UIElement* added = element.get();
	if (parent) parent->AddChild(std::move(element));
	else editingCanvas->AddElement(std::move(element));
	selectedUIElement = added;
}

UIElement* EngineUI::FindUIParent(UIElement* target) {
	//Depth-first through the saved children, the ones the hierarchy shows
	std::function<UIElement*(UIElement*)> Search = [&](UIElement* node) -> UIElement* {
		std::vector<UIElement*> children = node->GetChildren();
		children.resize(node->SavedChildCount());
		for (UIElement* child : children) {
			if (child == target) return node;
			if (UIElement* found = Search(child)) return found;
		}
		return nullptr;
	};
	for (UIElement* element : editingCanvas->GetElements()) {
		if (UIElement* found = Search(element)) return found;
	}
	return nullptr;
}

void EngineUI::DeleteUIElement() {
	if (!uiDeleteTarget) return;

	if (uiDeleteParent) uiDeleteParent->RemoveChild(uiDeleteTarget);
	else editingCanvas->RemoveElement(uiDeleteTarget);

	uiDeleteTarget = nullptr;
	uiDeleteParent = nullptr;
	selectedUIElement = nullptr;
}

void EngineUI::BuildUIPreview(Scene& scene) {
	//Scripts look for the player, so the builder keeps a stand-in
	auto player = std::make_unique<PlayerAgent>(instance);
	player->SetName("Preview Player");
	scene.AddAgent(std::move(player));

	scene.AddSystem(std::make_unique<UIManager>(instance));
}

void EngineUI::OpenCanvas(Scene& scene, UICanvas* canvas) {
	editingCanvas = canvas;
	selectedUIElement = nullptr;
	selectedAsset = canvas ? canvas->GetName() : "None";

	UIManager* ui = scene.GetSystem<UIManager>();
	if (!ui) return;

	std::vector<UICanvas*> shown;
	if (canvas) shown.push_back(canvas);
	ui->SetStateCanvases("Preview", shown);
	ui->SetState("Preview");
}
