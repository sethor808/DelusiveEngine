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

	availableContainer = nullptr;
	equippedContainer = nullptr;
	player = nullptr;
	inventoryData = nullptr;
	needsRebuild = true; //ReadyCheck resolves everything again
}

bool EquipScreen::ReadyCheck() {
	if (!rootElement) return false;

	UICanvas* canvas = rootElement->GetCanvas();
	if (!canvas) return false;
	if (!player) player = canvas->FetchPlayer();
	if (!player) return false;

	if (!inventoryData) inventoryData = player->GetInventory();
	if (!inventoryData) return false;

	if (!availableContainer) availableContainer = dynamic_cast<UIRepeatContainer*>(canvas->FindElementByUUID(availableContainerID.id));
	if (!equippedContainer) equippedContainer = dynamic_cast<UIRepeatContainer*>(canvas->FindElementByUUID(equippedContainerID.id));
	return availableContainer && equippedContainer;
}

void EquipScreen::OnInit() {
	needsRebuild = true;
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

	if (needsRebuild) Rebuild();
}

void EquipScreen::Rebuild() {
	needsRebuild = false;
	EnsureTemplate(availableContainer);
	EnsureTemplate(equippedContainer);
	BuildAvailableList();
	BuildEquippedSlots();

	//Optional, so it never blocks ReadyCheck
	UIElement* confirm = rootElement->GetCanvas()->FindElementByUUID(confirmButtonID.id);
	if (confirm && confirm->SupportsClick()) {
		confirm->SetOnClick([this]() { closeRequested = true; });
	}
}

//An empty container gets a plain button to copy, so the screen works before any styling
void EquipScreen::EnsureTemplate(UIRepeatContainer* container) {
	if (!container->GetChildren().empty()) return;

	auto* slot = container->AddChild<UIButton>();
	slot->SetID(UUID::GenerateRandom());
	slot->SetName("Slot");
	slot->SetSize(glm::vec2(style.iconSize * 1.2f));
	if (container->GetSpacing() <= 0.0f) container->SetSpacing(style.iconSize * 1.5f);
}

void EquipScreen::BuildAvailableList() {
	auto talismans = inventoryData->GetAvailableTalismans();
	availableContainer->SetCount(static_cast<int>(talismans.size()));

	auto children = availableContainer->GetChildren();
	for (size_t i = 0; i < children.size(); ++i) {
		UIElement* child = children[i];
		child->ClearChildren();
		if (i >= talismans.size()) continue;

		Talisman* talisman = talismans[i];
		BuildTalismanVisual(child, talisman, child->GetPosition(), style);
		if (child->SupportsClick()) child->SetOnClick([this, talisman]() { EquipToSlot(talisman); });
		else std::cerr << "[EquipScreen] " << child->GetName() << " can't be clicked - use a UIButton" << std::endl;
	}
}

void EquipScreen::BuildEquippedSlots() {
	auto equipped = inventoryData->GetEquippedTalismans();
	const int slots = static_cast<int>(equipped.size());
	equippedContainer->SetCount(slots);

	auto children = equippedContainer->GetChildren();
	for (int i = 0; i < slots && i < static_cast<int>(children.size()); ++i) {
		UIElement* child = children[i];
		child->ClearChildren();
		if (equipped[i]) BuildTalismanVisual(child, equipped[i], child->GetPosition(), style);

		//Empty slots are clickable too, to pick where the next talisman goes
		child->SetOnClick([this, i]() {
			inventoryData->UnequipTalisman(i);
			targetSlot = i;
			LoadoutChanged();
		});
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
	LoadoutChanged();
}

void EquipScreen::LoadoutChanged() {
	player->LoadFromInventory();
	needsRebuild = true;
}
