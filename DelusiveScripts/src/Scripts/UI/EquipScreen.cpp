#include <Scripts/UI/EquipScreen.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/UI/UIImage.h>
#include <Delusive/Runtime/Talismans/Talisman.h>

EquipScreen::EquipScreen()
    : UIScript()
{
    RegisterProperties();
}

void EquipScreen::RegisterProperties() {
    UIScript::RegisterProperties();

    registry->Register("availableContainerID", &availableContainerID);
    registry->Register("equippedContainerID", &equippedContainerID);
	registry->Register("confirmButtonID", &confirmButtonID);

	registry->Register("iconSize", &style.iconSize);
	registry->Register("stringSpacing", &style.stringSpacing);
	registry->Register("stringSize", &style.stringSize);
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
    //Make sure element is linked
    if (!rootElement) return false;

    //Make sure that player is findable
    UICanvas* canvas = rootElement->GetCanvas();
	if (!canvas) return false;
	if (!player) player = canvas->FetchPlayer();
    if (!player) return false;

    //Make sure link to Inventory works
    if (!inventoryData) {
        inventoryData = player->GetInventory();
        if (!inventoryData) {
            return false;
        }
    }
    
    //Check UUIDs for Necessary UI Elements
	if (!availableContainer) availableContainer = dynamic_cast<UIRepeatContainer*>(canvas->FindElementByUUID(availableContainerID.id));
	if (!equippedContainer) equippedContainer = dynamic_cast<UIRepeatContainer*>(canvas->FindElementByUUID(equippedContainerID.id));
	if (!availableContainer || !equippedContainer) return false;

    return true;
}

void EquipScreen::OnInit() {
	needsRebuild = true;
}

void EquipScreen::OnUpdate(float) {
	if (!rootElement || !rootElement->GetEnabled()) return;
	if (!ReadyCheck()) return;

	if (closeRequested) {
		closeRequested = false;
		player->LoadFromInventory();
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
	BuildAvailableList();
	BuildEquippedSlots();
	player->LoadFromInventory();

	//Optional, so it never blocks ReadyCheck
	UIElement* confirm = rootElement->GetCanvas()->FindElementByUUID(confirmButtonID.id);
	if (confirm && confirm->SupportsClick()) {
		confirm->SetOnClick([this]() { closeRequested = true; });
	}
}

void EquipScreen::BuildAvailableList() {
    auto talismans = inventoryData->GetAvailableTalismans();

    availableContainer->SetCount((int)talismans.size());
    availableContainer->RegenerateChildren();

    auto children = availableContainer->GetChildren();
	for (size_t i = 0; i < children.size() && i < talismans.size(); ++i) {
        Talisman* talisman = talismans[i];
        UIElement* child = children[i];

        child->ClearChildren();
        BuildTalismanVisual(child, talisman, child->GetPosition(), style);

        if (child->SupportsClick()) {
			child->SetOnClick([this, talisman]() { EquipToFirstOpenSlot(talisman); });
        }
        else {
            // Link to log here
        }
    }
}

void EquipScreen::BuildEquippedSlots() {
    int slots = inventoryData->GetSlotCount();
    auto equipped = inventoryData->GetEquippedTalismans();

    equippedContainer->SetCount(slots);
    equippedContainer->RegenerateChildren();

    auto children = equippedContainer->GetChildren();
	for (int i = 0; i < slots && i < static_cast<int>(children.size()); ++i) {
		UIElement* child = children[i];
		if (!equipped[i]) continue;

		child->ClearChildren();
		BuildTalismanVisual(child, equipped[i], child->GetPosition(), style);
		if (child->SupportsClick()) {
			child->SetOnClick([this, i]() {
				inventoryData->UnequipTalisman(i);
				needsRebuild = true;
			});
        }
    }
}

void EquipScreen::EquipToFirstOpenSlot(Talisman* talisman) {
    if (!inventoryData || !talisman) return;
    
    auto equipped = inventoryData->GetEquippedTalismans();
    int slotCount = inventoryData->GetSlotCount();

    for (int i = 0; i < slotCount; ++i) {
        if (!equipped[i]) {
            inventoryData->EquipTalisman(i, talisman);
			needsRebuild = true;
            return;
        }
    }
}