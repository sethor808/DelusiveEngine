#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_gamepad.h>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <imgui/imgui.h>
#include <sstream>

namespace {
	bool KeyDown(const PlayerInputState& input, const std::string& keyName) {
		if (!input.keys || keyName.empty()) return false;
		SDL_Scancode code = SDL_GetScancodeFromName(keyName.c_str());
		return code != SDL_SCANCODE_UNKNOWN && input.keys[code];
	}

	bool ButtonDown(const PlayerInputState& input, const std::string& buttonName) {
		if (buttonName.empty()) return false;
		SDL_GamepadButton button = SDL_GetGamepadButtonFromString(buttonName.c_str());
		return button != SDL_GAMEPAD_BUTTON_INVALID && (input.padButtons & (1u << button));
	}

	void CollectFocusTargets(UIElement* element, std::vector<UIElement*>& out) {
		if (!element->GetEnabled()) return;
		if (element->SupportsClick()) out.push_back(element);
		for (UIElement* child : element->GetChildren()) CollectFocusTargets(child, out);
	}

	void ClearFocusTree(UIElement* element) {
		element->SetFocused(false);
		for (UIElement* child : element->GetChildren()) ClearFocusTree(child);
	}

	UIElement* Nearest(const std::vector<UIElement*>& targets, const glm::vec2& pos) {
		UIElement* best = nullptr;
		float bestDistance = FLT_MAX;
		for (UIElement* target : targets) {
			glm::vec2 offset = target->GetPosition() - pos;
			float distance = glm::dot(offset, offset);
			if (distance < bestDistance) {
				bestDistance = distance;
				best = target;
			}
		}
		return best;
	}

	UIElement* TopLeft(const std::vector<UIElement*>& targets) {
		UIElement* best = targets.front();
		for (UIElement* target : targets) {
			const glm::vec2& pos = target->GetPosition();
			const glm::vec2& bestPos = best->GetPosition();
			bool higher = pos.y > bestPos.y + 0.01f;
			bool sameRowLeft = std::abs(pos.y - bestPos.y) <= 0.01f && pos.x < bestPos.x;
			if (higher || sameRowLeft) best = target;
		}
		return best;
	}

	//Closest target ahead in the direction, favouring ones in line with the current
	UIElement* Step(const std::vector<UIElement*>& targets, UIElement* from, const glm::vec2& dir) {
		glm::vec2 heading = glm::normalize(dir);
		UIElement* best = from;
		float bestScore = FLT_MAX;
		for (UIElement* target : targets) {
			glm::vec2 offset = target->GetPosition() - from->GetPosition();
			float along = glm::dot(offset, heading);
			if (target == from || along <= 0.01f) continue;

			float score = along + glm::length(offset - heading * along) * 2.0f;
			if (score < bestScore) {
				bestScore = score;
				best = target;
			}
		}
		return best;
	}
}

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
	registry->Register("stateButtons", &stateButtons);
	registry->Register("startState", &startState);
	//Edited through the States section of DrawImGui
	registry->Hide("stateNames");
	registry->Hide("stateCanvases");
	registry->Hide("statePauses");
	registry->Hide("stateKeys");
	registry->Hide("stateButtons");
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
		state.toggleButton = i < stateButtons.size() ? stateButtons[i] : "";

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
	stateButtons.clear();

	for (const UIState& state : states) {
		std::ostringstream ids;
		for (UICanvas* canvas : state.canvases) {
			ids << canvas->GetID().ToString() << " ";
		}
		stateNames.push_back(state.name);
		stateCanvases.push_back(ids.str());
		statePauses.push_back(state.pausesGame ? 1 : 0);
		stateKeys.push_back(state.toggleKey);
		stateButtons.push_back(state.toggleButton);
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
	hasFocus = false;
	ClearFocus();
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

std::vector<UICanvas*> UIManager::LinkCanvases() {
	std::vector<UICanvas*> canvases;

	for (int index : stack) {
		for (UICanvas* canvas : states[index].canvases) {
			canvas->LinkManager(this);
			canvases.push_back(canvas);
		}
	}

	return canvases;
}

std::vector<UICanvas*> UIManager::LinkInputCanvases() {
	std::vector<UICanvas*> canvases;

	//Display-only canvases (a HUD) let input fall through to the states below
	for (auto index = stack.rbegin(); index != stack.rend() && canvases.empty(); ++index) {
		for (UICanvas* canvas : states[*index].canvases) {
			if (!canvas->CapturesInput()) continue;
			canvas->LinkManager(this);
			canvases.push_back(canvas);
		}
	}

	return canvases;
}

bool UIManager::CapturesInput() const {
	for (int index : stack) {
		for (UICanvas* canvas : states[index].canvases) {
			if (canvas->CapturesInput()) return true;
		}
	}
	return false;
}

#pragma endregion

void UIManager::Update(float deltaTime) {
	for (UICanvas* canvas : LinkCanvases()) {
		canvas->Update(deltaTime);
	}
}

void UIManager::Draw(const glm::mat4& projection) {
	for (UICanvas* canvas : LinkCanvases()) {
		canvas->Draw(projection);
	}
}

void UIManager::HandleMouse(const glm::vec2& mousePos, bool mouseDown) {
	if (mousePos != lastMouse || mouseDown) {
		lastMouse = mousePos;
		if (padFocus) {
			padFocus = false;
			ClearFocus();
		}
	}

	if (waitForRelease) {
		if (mouseDown) return;
		waitForRelease = false;
	}

	const std::vector<int> before = stack;
	for (UICanvas* canvas : LinkInputCanvases()) {
		canvas->HandleMouse(mousePos, mouseDown);
		//A button changed the stack - the old canvases are done
		if (stack != before) return;
	}
}

void UIManager::HandleInput(const PlayerInputState& input) {
	int toggled = -1;
	for (size_t i = 0; i < states.size(); ++i) {
		UIState& state = states[i];
		bool padDown = ButtonDown(input, state.toggleButton);
		bool down = padDown || KeyDown(input, state.toggleKey);
		if (down && !state.toggleWasDown) {
			toggled = static_cast<int>(i);
			//Opened from the gamepad, so focus shows straight away
			padFocus = padDown;
		}
		state.toggleWasDown = down;
	}

	if (toggled >= 0) {
		if (!stack.empty() && stack.back() == toggled) PopState();
		else PushState(states[toggled].name);
	}
	else {
		Navigate(input);
	}

	for (UICanvas* canvas : LinkInputCanvases()) {
		canvas->HandleInput(input);
	}
}

void UIManager::Navigate(const PlayerInputState& input) {
	std::vector<UICanvas*> canvases = LinkInputCanvases();
	if (canvases.empty()) return;

	if (input.backPressed) {
		PopState();
		return;
	}

	std::vector<UIElement*> targets;
	for (UICanvas* canvas : canvases) {
		for (UIElement* element : canvas->GetElements()) CollectFocusTargets(element, targets);
	}
	if (targets.empty()) return;

	const bool navigated = input.navDir != glm::vec2(0.0f);
	//The first press only shows where focus is
	const bool shown = padFocus && hasFocus;
	if (navigated || input.confirmPressed) padFocus = true;
	if (!padFocus) return;

	UIElement* focus = hasFocus ? Nearest(targets, focusPos) : TopLeft(targets);
	if (navigated && shown) focus = Step(targets, focus, input.navDir);
	focusPos = focus->GetPosition();
	hasFocus = true;

	for (UIElement* target : targets) target->SetFocused(target == focus);

	if (input.confirmPressed && shown) focus->Click();
}

void UIManager::ClearFocus() {
	for (const UIState& state : states) {
		for (UICanvas* canvas : state.canvases) {
			for (UIElement* element : canvas->GetElements()) ClearFocusTree(element);
		}
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

	std::snprintf(buffer, sizeof(buffer), "%s", state.toggleButton.c_str());
	if (ImGui::InputTextWithHint("Toggle button", "Gamepad, e.g. back, start", buffer, sizeof(buffer))) {
		state.toggleButton = buffer;
	}
	if (!state.toggleButton.empty() && SDL_GetGamepadButtonFromString(state.toggleButton.c_str()) == SDL_GAMEPAD_BUTTON_INVALID) {
		ImGui::TextDisabled("Unknown button name");
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
