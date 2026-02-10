#include <DelusiveScripts/UI/EquipScreen.h>
#include <DelusiveExternal/DelusiveTalismanData.h>
#include <DelusiveExternal/DelusiveScriptPlayer.h>
#include <DelusiveExternal/DelusiveUICanvas.h>
#include <DelusiveExternal/DelusiveUIScriptContainer.h>
#include <DelusiveExternal/DelusiveUIButton.h>

bool EquipScreen::ReadyCheck() {
    //Make sure element is linked
    if (!rootElement) return false;

    //Make sure that player is findable
    auto canvas = rootElement->GetCanvas();
    auto player = canvas.GetPlayer();
    if (!player.IsValid()) return false;

    //Make sure link to Inventory works
    if (!inventoryData.IsValid()) {
        inventoryData = player.GetInventory();
        if (!inventoryData.IsValid()) {
            return false;
        }
    }

    
    
    
    //Check UUIDs for Necessary UI Elements
    DelusiveUIElement availableLink = canvas.FindElementByUUID(availableContainerID);
    if (availableLink.IsValid()) {
        availableContainer.Link(availableLink.GetLink());
    }
    else { return false; }
    DelusiveUIElement equippedLink = canvas.FindElementByUUID(equippedContainerID);
    if (equippedLink.IsValid()) {
        equippedContainer.Link(equippedLink.GetLink());
    }
    else { return false; }

    if (!rootElement->HasBinding("AvailableList")) {
        rootElement->SetBinding("AvailableList", availableContainerID);
    }
    if (!rootElement->HasBinding("EquippedSlots")) {
        rootElement->SetBinding("EquippedSlots", equippedContainerID);
    }

    if (!availableContainer.IsValid()) return false;
    if (!equippedContainer.IsValid()) return false;

    return true;
}

void EquipScreen::OnInit() {
    DelusiveScriptPlayer player = rootElement->GetCanvas().GetPlayer();
}

void EquipScreen::BuildAvailableList() {
    auto talismans = inventoryData.GetAvailableTalismans();

    availableContainer.SetCount((int)talismans.size());
    availableContainer.Regenerate();

    auto children = availableContainer.GetChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        DelusiveTalismanData talisman = talismans[i];
        DelusiveUIElement child(children[i]);

        if (child.SupportsClick()) {
            child.SetOnClick([this, talisman]() {
                EquipToFirstOpenSlot(talisman);
                });
        }
        else {
            // Link to log here
        }
    }
}

void EquipScreen::BuildEquippedSlots() {
    int slots = inventoryData.GetSlotCount();
    auto equipped = inventoryData.GetEquippedTalismans();

    equippedContainer.SetCount(slots);
    equippedContainer.Regenerate();

    auto children = equippedContainer.GetChildren();

    for (int i = 0; i < slots; ++i) {
        DelusiveUIElement child(children[i]);

        if (equipped[i].IsValid()) {
            if (child.SupportsClick()) {
                const int slotIndex = i;
                child.SetOnClick([this, slotIndex]() {
                    inventoryData.UnequipTalisman(slotIndex);

                    // Refresh UI
                    BuildAvailableList();
                    BuildEquippedSlots();
                    });
            }
        }
        else {
            // Maybe disable click here
            if (child.SupportsClick()) {
                //child.ClearOnClick();
            }
        }
    }
}

void EquipScreen::EquipToFirstOpenSlot(const DelusiveTalismanData& talisman) {
    if (!inventoryData.IsValid() || !talisman.IsValid()) return;
    
    auto equipped = inventoryData.GetEquippedTalismans();
    int slotCount = inventoryData.GetSlotCount();

    for (int i = 0; i < slotCount; ++i) {
        if (!equipped[i].IsValid()) {
            inventoryData.EquipTalisman(i, talisman);

            // Refresh UI
            BuildAvailableList();
            BuildEquippedSlots();
            return;
        }
    }
}