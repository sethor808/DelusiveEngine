#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <SDL3/SDL_keyboard.h>
#include <algorithm>
#include <iostream>
#include <imgui/imgui.h>
#include <sstream>

UIManager::UIManager(DelusiveInstance& instance)
	: SceneSystem(instance)
{
	name = "NewUIManager";
	RegisterProperties();
}

UIManager::~UIManager() {
	//Canvases outlive the scene, so they must not keep pointing here
	for (UIState& state : states) {
		for (UICanvas* canvas : state.canvases) {
			if (canvas->GetManager() == this) canvas->DelinkManager();
		}
	}
}

void UIManager::Init() {
	//Runs after Deserialize, so the state lists are already loaded
	ResolveStates();
}

void UIManager::LinkScene(Scene* _scene) {
	scene = _scene;
}

ScriptManager& UIManager::GetScriptManager() const {
	if (!scene)
		throw std::runtime_error("UIManager: scene null");

	if (!scene->HasGameManager())
		throw std::runtime_error("UIManager: gameManager null");

	return scene->GetScriptManager();
}

void UIManager::RegisterProperties() {
	SceneSystem::RegisterProperties();
	registry->Register("stateNames", &stateNames);
	registry->Register("stateCanvases", &stateCanvases);
	registry->Register("statePauses", &statePauses);
	registry->Register("stateKeys", &stateKeys);
	registry->Register("startState", &startState);
	//Edited through the States section of DrawImGui
	registry->Hide("stateNames");
	registry->Hide("stateCanvases");
	registry->Hide("statePauses");
	registry->Hide("stateKeys");
	registry->Hide("startState");
}

DelusiveUIRegistry* UIManager::FetchRegistry() const {
	if (!scene || !scene->GetGameManager()) return nullptr;
	return &scene->GetGameManager()->GetUIRegistry();
}

#pragma region States

int UIManager::FindState(const std::string& stateName) const {
	for (size_t i = 0; i < states.size(); ++i) {
		if (states[i].name == stateName) return static_cast<int>(i);
	}
	return -1;
}

void UIManager::ResolveStates() {
	states.clear();
	stack.clear();

	DelusiveUIRegistry* uiRegistry = FetchRegistry();
	if (!uiRegistry) return;

	for (size_t i = 0; i < stateNames.size(); ++i) {
		UIState state;
		state.name = stateNames[i];
		state.pausesGame = i < statePauses.size() && statePauses[i] != 0;
		state.toggleKey = i < stateKeys.size() ? stateKeys[i] : "";

		std::istringstream ids(i < stateCanvases.size() ? stateCanvases[i] : "");
		std::string idText;
		while (ids >> idText) {
			UUID canvasID;
			canvasID.FromString(idText);

			if (UICanvas* canvas = uiRegistry->Get(canvasID)) {
				state.canvases.push_back(canvas);
			}
			else {
				std::cerr << "[UIManager] Missing canvas " << idText << " in state " << state.name << std::endl;
			}
		}

		states.push_back(std::move(state));
	}

	if (states.empty()) return;
	SetState(startState.empty() ? states.front().name : startState);
}

void UIManager::SyncStates() {
	stateNames.clear();
	stateCanvases.clear();
	statePauses.clear();
	stateKeys.clear();

	for (const UIState& state : states) {
		std::ostringstream ids;
		for (UICanvas* canvas : state.canvases) {
			ids << canvas->GetID().ToString() << " ";
		}
		stateNames.push_back(state.name);
		stateCanvases.push_back(ids.str());
		statePauses.push_back(state.pausesGame ? 1 : 0);
		stateKeys.push_back(state.toggleKey);
	}
}

void UIManager::StackChanged() {
	for (const UIState& state : states) {
		for (UICanvas* canvas : state.canvases) canvas->SetActive(false);
	}
	for (int index : stack) {
		for (UICanvas* canvas : states[index].canvases) canvas->SetActive(true);
	}

	waitForRelease = true;
}

void UIManager::SetState(const std::string& stateName) {
	int index = FindState(stateName);
	if (index < 0) {
		std::cerr << "[UIManager] No UI state named " << stateName << std::endl;
		return;
	}

	stack = { index };
	StackChanged();
}

void UIManager::PushState(const std::string& stateName) {
	int index = FindState(stateName);
	if (index < 0) {
		std::cerr << "[UIManager] No UI state named " << stateName << std::endl;
		return;
	}
	if (std::find(stack.begin(), stack.end(), index) != stack.end()) return;

	stack.push_back(index);
	StackChanged();
}

void UIManager::PopState() {
	if (stack.size() <= 1) return;

	stack.pop_back();
	StackChanged();
}

const std::string& UIManager::GetState() const {
	static const std::string none;
	return stack.empty() ? none : states[stack.back()].name;
}

bool UIManager::IsPaused() const {
	for (int index : stack) {
		if (states[index].pausesGame) return true;
	}
	return false;
}

void UIManager::SetStateCanvases(const std::string& stateName, const std::vector<UICanvas*>& canvases) {
	int index = FindState(stateName);

	if (index < 0) {
		UIState state;
		state.name = stateName;
		state.canvases = canvases;
		states.push_back(std::move(state));
	}
	else {
		states[index].canvases = canvases;
		StackChanged();
	}

	SyncStates();
}

std::vector<UICanvas*> UIManager::LinkCanvases(bool topOnly) {
	std::vector<UICanvas*> canvases;

	for (size_t i = topOnly && !stack.empty() ? stack.size() - 1 : 0; i < stack.size(); ++i) {
		for (UICanvas* canvas : states[stack[i]].canvases) {
			canvas->LinkManager(this);
			canvases.push_back(canvas);
		}
	}

	return canvases;
}

#pragma endregion

void UIManager::Update(float deltaTime) {
	for (UICanvas* canvas : LinkCanvases(false)) {
		canvas->Update(deltaTime);
	}
}

void UIManager::Draw(const glm::mat4& projection) {
	for (UICanvas* canvas : LinkCanvases(false)) {
		canvas->Draw(projection);
	}
}

void UIManager::HandleMouse(const glm::vec2& mousePos, bool mouseDown) {
	if (waitForRelease) {
		if (mouseDown) return;
		waitForRelease = false;
	}

	const std::vector<int> before = stack;
	for (UICanvas* canvas : LinkCanvases(true)) {
		canvas->HandleMouse(mousePos, mouseDown);
		//A button changed the stack - the old top is done
		if (stack != before) return;
	}
}

void UIManager::HandleInput(const PlayerInputState& input) {
	if (input.keys) {
		int toggled = -1;
		for (size_t i = 0; i < states.size(); ++i) {
			UIState& state = states[i];
			if (state.toggleKey.empty()) continue;

			SDL_Scancode code = SDL_GetScancodeFromName(state.toggleKey.c_str());
			if (code == SDL_SCANCODE_UNKNOWN) continue;

			bool down = input.keys[code];
			if (down && !state.keyWasDown) toggled = static_cast<int>(i);
			state.keyWasDown = down;
		}

		if (toggled >= 0) {
			if (!stack.empty() && stack.back() == toggled) PopState();
			else PushState(states[toggled].name);
		}
	}

	for (UICanvas* canvas : LinkCanvases(true)) {
		canvas->HandleInput(input);
	}
}

void UIManager::DrawImGui() {
	DelusiveUIRegistry* uiRegistry = FetchRegistry();
	if (!uiRegistry) {
		ImGui::TextDisabled("Not in a scene");
		return;
	}

	if (ImGui::Button("Save Canvases")) {
		uiRegistry->SaveAll();
	}

	ImGui::SeparatorText("States");

	if (ImGui::BeginCombo("Start state", startState.empty() ? "<First>" : startState.c_str())) {
		if (ImGui::Selectable("<First>", startState.empty())) startState.clear();
		for (const UIState& state : states) {
			if (ImGui::Selectable(state.name.c_str(), state.name == startState)) startState = state.name;
		}
		ImGui::EndCombo();
	}

	std::string stackText;
	for (int index : stack) stackText += (stackText.empty() ? "" : " > ") + states[index].name;
	ImGui::TextDisabled("Stack: %s", stackText.empty() ? "<empty>" : stackText.c_str());

	int removeIndex = -1;
	for (size_t i = 0; i < states.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));
		if (ImGui::TreeNodeEx("##state", ImGuiTreeNodeFlags_DefaultOpen, "%s", states[i].name.c_str())) {
			DrawStateImGui(states[i], *uiRegistry);

			if (ImGui::SmallButton("Set")) SetState(states[i].name);
			ImGui::SameLine();
			if (ImGui::SmallButton("Push")) PushState(states[i].name);
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove")) removeIndex = static_cast<int>(i);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if (removeIndex >= 0) {
		//Stack entries are indices, so drop the removed one and shift the rest
		stack.erase(std::remove(stack.begin(), stack.end(), removeIndex), stack.end());
		for (int& index : stack) {
			if (index > removeIndex) --index;
		}
		for (UICanvas* canvas : states[removeIndex].canvases) canvas->SetActive(false);
		states.erase(states.begin() + removeIndex);
		StackChanged();
	}

	if (ImGui::Button("Add State")) {
		UIState state;
		state.name = "State " + std::to_string(states.size() + 1);
		states.push_back(std::move(state));
	}

	SyncStates();
}

void UIManager::DrawStateImGui(UIState& state, DelusiveUIRegistry& uiRegistry) {
	char buffer[64];
	std::snprintf(buffer, sizeof(buffer), "%s", state.name.c_str());
	if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
		if (startState == state.name) startState = buffer;
		state.name = buffer;
	}

	ImGui::Checkbox("Pauses game", &state.pausesGame);

	std::snprintf(buffer, sizeof(buffer), "%s", state.toggleKey.c_str());
	if (ImGui::InputTextWithHint("Toggle key", "e.g. Escape, Tab", buffer, sizeof(buffer))) {
		state.toggleKey = buffer;
	}
	if (!state.toggleKey.empty() && SDL_GetScancodeFromName(state.toggleKey.c_str()) == SDL_SCANCODE_UNKNOWN) {
		ImGui::TextDisabled("Unknown key name");
	}

	//Every canvas, ticked when this state shows it
	for (UICanvas* canvas : uiRegistry.List()) {
		auto found = std::find(state.canvases.begin(), state.canvases.end(), canvas);
		bool shown = found != state.canvases.end();

		ImGui::PushID(canvas);
		if (ImGui::Checkbox(canvas->GetName().c_str(), &shown)) {
			if (shown) state.canvases.push_back(canvas);
			else state.canvases.erase(found);
			StackChanged();
		}
		ImGui::PopID();
	}
}

void UIManager::Reset() {
	states.clear();
	stack.clear();
	SyncStates();
}
