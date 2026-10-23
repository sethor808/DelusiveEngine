#include <Delusive/Runtime/Editor/EngineUI.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Collision/HitData.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <imgui/imgui.h>

void EngineUI::RenderUIBuilder(Scene& scene) {
	ImGui::Begin(UIBuilderPanelWindow);

	//Nothing updates the scene outside play, so the preview ticks here
	UIManager* ui = scene.GetSystem<UIManager>();
	if (ui) ui->Update(ImGui::GetIO().DeltaTime);

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

	ImGui::SameLine();
	//Each canvas goes back to its own .canvas file; new ones get CANVAS_PATH/<name>
	if (ImGui::Button("Save All")) {
		gameManager.GetUIRegistry().SaveAll();
	}

	ImGui::Separator();

	if (!editingCanvas) {
		ImGui::TextDisabled("Pick or add a canvas from Loaded Asset.");
		ImGui::End();
		return;
	}

	// Hierarchy
	if (ImGui::BeginChild("UIHierarchy", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true)) {
		ImGui::Text("%s", editingCanvas->GetName().c_str());
		bool captures = editingCanvas->CapturesInput();
		if (ImGui::Checkbox("Captures input", &captures)) editingCanvas->SetCapturesInput(captures);
		ImGui::SetItemTooltip("Menus take clicks and keys from the game; leave off for HUDs");

		if (ImGui::BeginPopupContextWindow("CanvasRightClick", ImGuiPopupFlags_MouseButtonRight)) {
			std::string type = DelusiveUI::DrawUIElementAddMenu();
			if (!type.empty()) {
				auto element = DelusiveUI::CreateUIElementByType(type, instance);
				if (element) {
					element->LinkCanvas(editingCanvas);
					editingCanvas->AddElement(std::move(element));
				}
			}
			ImGui::EndPopup();
		}

		ImGui::Separator();
		for (UIElement* element : editingCanvas->GetElements()) {
			DrawUIElementNode(element);
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Viewport (Stage 2: render canvas to framebuffer + spatial drag/resize)
	if (ImGui::BeginChild("UIViewport", ImVec2(ImGui::GetContentRegionAvail().x * 0.55f, 0), true)) {
		ImGui::TextDisabled("Viewport");
		ImGui::TextDisabled("Spatial editing lands in Stage 2.");
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Inspector (reuses the registered-property DrawImGui path)
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

void EngineUI::DrawUIElementNode(UIElement* element) {
	if (!element) return;

	std::vector<UIElement*> children = element->GetChildren();

	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		(children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) |
		(selectedUIElement == element ? ImGuiTreeNodeFlags_Selected : 0);

	std::string label = element->GetName().empty() ? element->GetType() : element->GetName();

	ImGui::PushID(element);
	bool open = ImGui::TreeNodeEx(label.c_str(), flags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		selectedUIElement = element;
	}

	if (ImGui::BeginPopupContextItem("ElementRightClick", ImGuiPopupFlags_MouseButtonRight)) {
		std::string type = DelusiveUI::DrawUIElementAddMenu();
		if (!type.empty()) {
			auto child = DelusiveUI::CreateUIElementByType(type, instance);
			if (child) {
				child->LinkCanvas(editingCanvas);
				element->AddChild(std::move(child));
			}
		}
		ImGui::EndPopup();
	}

	if (open) {
		for (UIElement* child : children) {
			DrawUIElementNode(child);
		}
		ImGui::TreePop();
	}

	ImGui::PopID();
}

void EngineUI::BuildUIPreview(Scene& scene) {
	//Canvases bind to game data, so the builder keeps a stand-in player
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
