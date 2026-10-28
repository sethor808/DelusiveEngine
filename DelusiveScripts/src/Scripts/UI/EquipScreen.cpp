#include <Scripts/UI/EquipScreen.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIButton.h>
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <algorithm>
#include <iostream>

EquipScreen::EquipScreen()
	: UIScript() {
	RegisterProperties();
}

void EquipScreen::RegisterProperties() {
	UIScript::RegisterProperties();

	registry->Register("availableContainerID", &availableContainerID);
	registry->Register("equippedContainerID", &equippedContainerID);
	registry->Register("confirmButtonID", &confirmButtonID);

	registry->Register("iconSize", &style.iconSize);
	registry->Register("stringSpacing", &style.stringSpacing);
	registry->Register("stringXOffset", &style.stringXOffset);
	registry->Register("stringYOffset", &style.stringYOffset);
}

void EquipScreen::Link(UIScriptContainer* root) {
	UIScript::Link(root);
	availableContainerID.canvasLink = root->GetCanvas();
	equippedContainerID.canvasLink = root->GetCanvas();
	confirmButtonID.canvasLink = root->GetCanvas();
}

void EquipScreen::RelocateReferences() {
	UIScript::RelocateReferences();
	builtFrom.clear();
}

//Looked up every frame: the canvas is shared, so the player changes between the builder and play
bool EquipScreen::ReadyCheck() {
	if (!rootElement) return false;

	UICanvas* canvas = rootElement->GetCanvas();
	if (!canvas) return false;
	player = canvas->FetchPlayer();
	inventoryData = player ? player->GetInventory() : nullptr;
	if (!inventoryData) return false;

	availableContainer = dynamic_cast<UIRepeatContainer*>(canvas->FindElementByUUID(availableContainerID.id));
	equippedContainer = dynamic_cast<UIRepeatContainer*>(canvas->FindElementByUUID(equippedContainerID.id));
	return availableContainer && equippedContainer;
}

void EquipScreen::OnUpdate(float) {
	if (!rootElement || !rootElement->GetEnabled()) return;
	if (!ReadyCheck()) return;

	if (closeRequested) {
		closeRequested = false;
		//Inside a UI state the screen closes by popping it; otherwise it just hides
		UIManager* manager = rootElement->GetCanvas()->GetManager();
		if (manager) manager->PopState();
		else rootElement->SetEnabled(false);
		return;
	}

	EnsureTemplate(availableContainer);
	EnsureTemplate(equippedContainer);

	auto available = inventoryData->GetAvailableTalismans();
	auto equipped = inventoryData->GetEquippedTalismans();
	availableContainer->SetCount(static_cast<int>(available.size()));
	equippedContainer->SetCount(static_cast<int>(equipped.size()));

	//Rebuilds only when something shown changes, including slots moved in the builder
	std::string current = std::to_string(style.iconSize) + "," + std::to_string(style.stringSpacing) + ","
		+ std::to_string(style.stringXOffset) + "," + std::to_string(style.stringYOffset);
	Describe(current, availableContainer, available);
	Describe(current, equippedContainer, equipped);
	if (current == builtFrom) return;
	builtFrom = std::move(current);

	rootElement->ClearChildren();
	BuildSlots(availableContainer, available, [this](int, Talisman* talisman) { EquipToSlot(talisman); });
	BuildSlots(equippedContainer, equipped, [this](int slot, Talisman*) {
		inventoryData->UnequipTalisman(slot);
		targetSlot = slot;
		player->LoadFromInventory();
	});

	//Optional, so it never blocks ReadyCheck
	UIElement* confirm = rootElement->GetCanvas()->FindElementByUUID(confirmButtonID.id);
	if (confirm && confirm->SupportsClick()) {
		confirm->SetOnClick([this]() { closeRequested = true; });
	}
}

//An empty container gets a plain button as its template, which the builder can then restyle
void EquipScreen::EnsureTemplate(UIRepeatContainer* container) {
	if (!container->GetChildren().empty()) return;

	auto* slot = container->AddChild<UIButton>();
	slot->SetID(UUID::GenerateRandom());
	slot->SetName("Slot");
	slot->SetSize(glm::vec2(style.iconSize * 1.2f));
	if (container->GetSpacing() <= 0.0f) container->SetSpacing(style.iconSize * 1.5f);
}

void EquipScreen::Describe(std::string& text, UIRepeatContainer* container, const std::vector<Talisman*>& talismans) {
	auto slots = container->GetChildren();
	const glm::vec2& size = slots.front()->GetSize();
	text += "|" + std::to_string(size.x) + "," + std::to_string(size.y);

	for (size_t i = 0; i < talismans.size() && i < slots.size(); ++i) {
		const glm::vec2& position = slots[i]->GetPosition();
		text += ";" + std::to_string(position.x) + "," + std::to_string(position.y) + ":"
			+ (talismans[i] ? talismans[i]->GetID().ToString() : "-");
	}
}

void EquipScreen::BuildSlots(UIRepeatContainer* container, const std::vector<Talisman*>& talismans, const std::function<void(int, Talisman*)>& onClick) {
	auto slots = container->GetChildren();
	//Copies keep up with size changes made to the template
	const glm::vec2 size = slots.front()->GetSize();

	for (int i = 0; i < static_cast<int>(talismans.size()) && i < static_cast<int>(slots.size()); ++i) {
		UIElement* slot = slots[i];
		Talisman* talisman = talismans[i];

		slot->SetSize(size);
		if (talisman) BuildTalismanVisual(rootElement, talisman, slot->GetPosition(), style);

		if (slot->SupportsClick()) slot->SetOnClick([onClick, i, talisman]() { onClick(i, talisman); });
		else std::cerr << "[EquipScreen] " << slot->GetName() << " can't be clicked - use a UIButton" << std::endl;
	}
}

void EquipScreen::EquipToSlot(Talisman* talisman) {
	auto equipped = inventoryData->GetEquippedTalismans();

	int slot = targetSlot;
	if (slot < 0 || slot >= static_cast<int>(equipped.size()) || equipped[slot]) {
		auto empty = std::find(equipped.begin(), equipped.end(), nullptr);
		if (empty == equipped.end()) return;
		slot = static_cast<int>(empty - equipped.begin());
	}

	inventoryData->EquipTalisman(slot, talisman);
	targetSlot = -1;
	player->LoadFromInventory();
}
