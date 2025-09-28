#include "UITalismanDisplay.h"
#include "PlayerAgent.h"
#include "DelusiveRegistry.h"

UITalismanDisplay::UITalismanDisplay(DelusiveRenderer& _renderer)
	: UIElement(_renderer), player(nullptr)
{
	LinkPlayer();
}

void UITalismanDisplay::LinkPlayer(PlayerAgent* _player) {
	player = player;

	if (player == nullptr) {
		// Attempt to find the player in the scene or game context
		// This is a placeholder; actual implementation will depend on your game's architecture
		// this->player = GameContext::GetInstance().GetPlayerAgent();
	}
}

std::unique_ptr<UIElement> UITalismanDisplay::Clone() const {
	auto copy = std::make_unique<UITalismanDisplay>(renderer);
	copy->SetPosition(position);
	copy->SetName(name);
	copy->SetEnabled(enabled);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UITalismanDisplay::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("leftOffset", &leftOffset);
	registry->Register("topOffset", &topOffset);
}

void UITalismanDisplay::Update(float deltaTime) {
	//Add updates here
	UIElement::Update(deltaTime);
}

void UITalismanDisplay::Draw(const glm::mat4& proj) {
	if (!player || !enabled) return;

	int winW, winH;
	renderer.GetWindowSize(winW, winH);


	// Top-left screen in world coords
	float startX = -winW / 2.0f + leftOffset;
	float startY = winH / 2.0f - topOffset;

	const auto& talismans = player->GetTalismans();

	//Add draw calls here
	UIElement::Draw(proj);
}