#include <DelusiveInternal/UI/UITalismanDisplay.h>
#include <DelusiveInternal/Agents/PlayerAgent.h>
#include <DelusiveInternal/Core/DelusiveData.h>
#include <DelusiveInternal/Core/DelusiveRegistry.h>
#include <DelusiveInternal/Talismans/Talisman.h>
#include <DelusiveInternal/UI/UICanvas.h>

UITalismanDisplay::UITalismanDisplay(DelusiveRenderer& _renderer)
	: UIElement(_renderer), player(nullptr)
{
	LinkPlayer();
	RegisterProperties();
}

void UITalismanDisplay::LinkPlayer(PlayerAgent* _player) {
	player = _player;

	if (player != nullptr || !parentCanvas) return;

	//Attempt to find player
	player = parentCanvas->FetchPlayer();
	if (player == nullptr) {
		std::cout << "[UITalismanDisplay] Warning: Could not find PlayerAgent to link to." << std::endl;
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
	registry->Register("iconSize", &iconSize);
	registry->Register("spacing", &spacing);
	registry->Register("stringSize", &stringSize);
	registry->Register("stringXOffset", &stringXOffset);
	registry->Register("stringYOffset", &stringYOffset);
	registry->Register("stringSpacing", &stringSpacing);
}

void UITalismanDisplay::Update(float deltaTime) {
	//Add updates here
	UIElement::Update(deltaTime);
}

//TODO: Adjust this so that the draggers work within pixel units for better fidelity
void UITalismanDisplay::Draw(const glm::mat4& proj) {
	if (!player) LinkPlayer();
	if (!player || !enabled) return;

	int winW, winH;
	renderer.GetWindowSize(winW, winH);

	// starting offset (top-left corner of the screen + configured offsets)
	float startX = (-winW / 2.0f + leftOffset) / DELUSIVE_PIXEL_SCALE;
	float startY = (winH / 2.0f - topOffset) / DELUSIVE_PIXEL_SCALE;
	glm::vec2 start = { startX, startY };

	const auto& talismans = player->GetTalismans();

	for (size_t i = 0; i < talismans.size(); i++) {
		const auto& t = talismans[i];
		if (t->GetIsBroken()) continue;

		GLuint tex = renderer.GetTexture(t->GetBaseTexture());
		if (!tex) continue;

		// Position in UI units
		glm::vec2 posUI = start + glm::vec2(i * spacing, 0.0f);

		// Convert to world coords
		glm::vec2 worldPos = posUI * DELUSIVE_PIXEL_SCALE;
		glm::vec2 worldSize = glm::vec2(iconSize, iconSize) * DELUSIVE_PIXEL_SCALE;

		glm::mat4 model =
			glm::translate(glm::mat4(1.0f), glm::vec3(worldPos, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(worldSize, 1.0f));

		renderer.Submit({
			.modelMatrix = model,
			.color = glm::vec4(1.0f),
			.textureID = tex,
			.layer = 100,
			.isUI = true
			});

		int currentHP = t->GetCurrentHP();
		if (currentHP <= 0) continue;

		GLuint stringTex = renderer.GetTexture(t->GetStringTexture());
		if (!stringTex) {
			std::cout << "[UITalismanDisplay] Missing string texture.\n";
			continue;
		}

		int maxHP = t->GetMaxHP();
		float stringXStart = (stringSpacing * 2) / maxHP;
		glm::vec2 stringStart = posUI + glm::vec2(-stringXStart, stringYOffset);
		glm::vec2 stringWorldSize = glm::vec2(stringSize) * DELUSIVE_PIXEL_SCALE;
		
		// --- Draw each string ---
		for (int s = 0; s < currentHP; ++s) {
			// --- Compute string world size ---
			glm::vec2 stringPos = (stringStart + glm::vec2(stringSpacing * s + stringXOffset, 0)) * DELUSIVE_PIXEL_SCALE; // start Y distance above talisman
			
			glm::mat4 stringModel =
				glm::translate(glm::mat4(1.0f), glm::vec3(stringPos, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(stringWorldSize, 1.0f));

			renderer.Submit({
				.modelMatrix = stringModel,
				.color = glm::vec4(1.0f),
				.textureID = stringTex,
				.layer = 101,
				.isUI = true
				});
		}
	}

	// draw children if any
	UIElement::Draw(proj);
}