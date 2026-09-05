#include <Scripts/UI/EquipScreen.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/UI/UIImage.h>

EquipScreen::EquipScreen()
    : UIScript()
{
    RegisterProperties();
}

void EquipScreen::RegisterProperties() {
    UIScript::RegisterProperties();

    registry->Register("availableContainerID", &availableContainerID);
    registry->Register("equippedContainerID", &equippedContainerID);
}

void EquipScreen::Link(UIScriptContainer* root) {
    UIScript::Link(root);
    availableContainerID.canvasLink = root->GetCanvas();
    equippedContainerID.canvasLink = root->GetCanvas();
}

void EquipScreen::RelocateReferences() {
    UIScript::RelocateReferences();

    availableContainer = nullptr;
    equippedContainer = nullptr;

    if (availableContainerID.id.IsValid() && rootElement) {
        availableContainer = dynamic_cast<UIRepeatContainer*>(rootElement->GetCanvas()->FindElementByUUID(availableContainerID.id));
    }
    else {
        availableContainer = nullptr;
    }

    if (equippedContainerID.id.IsValid() && rootElement) {
        equippedContainer = dynamic_cast<UIRepeatContainer*>(rootElement->GetCanvas()->FindElementByUUID(equippedContainerID.id));
    }
    else {
        equippedContainer = nullptr;
    }
}

bool EquipScreen::ReadyCheck() {
    //Make sure element is linked
    if (!rootElement) return false;

    //Make sure that player is findable
    UICanvas* canvas = rootElement->GetCanvas();
    PlayerAgent* player = canvas->FetchPlayer();
    if (!player) return false;

    //Make sure link to Inventory works
    if (!inventoryData) {
        inventoryData = player->GetInventory();
        if (!inventoryData) {
            return false;
        }
    }
    
    //Check UUIDs for Necessary UI Elements
    if (!availableContainer) {
        availableContainer = static_cast<UIRepeatContainer*>(canvas->FindElementByUUID(availableContainerID.id));
        if (!availableContainer) return false;
    }
    if (!equippedContainer) {
        equippedContainer = static_cast<UIRepeatContainer*>(canvas->FindElementByUUID(equippedContainerID.id));
        if (!equippedContainer) return false;
    }

    return true;
}

void EquipScreen::OnInit() {
    //DelusiveScriptPlayer player = rootElement->GetCanvas().FetchPlayer();
}

void EquipScreen::BuildTalismanVisual(UIElement* root, Talisman* talisman) {
    if (!root || !talisman) return;

    root->ClearChildren();
    

    //Build base
    auto* base = root->AddChild<UIImage>();
    base->SetName("TalismanBase");
    base->SetTexturePath(talisman->GetBaseTexture());
    base->SetSize({ iconSize, iconSize });
    base->SetPosition({ 0.0f, 0.0f });

    //Build glyph
    auto* glyph = base->AddChild<UIImage>();
    glyph->SetName("TalismanGlyph");
    glyph->SetTexturePath(talisman->GetGlyphTexture());
    glyph->SetSize({ iconSize, iconSize });
    glyph->SetPosition({ 0.0f, 0.0f }); // centered relative to base

    //Build strings
    int currentHP = talisman->GetCurrentHP();
    int maxHP = talisman->GetMaxHP();

    if (currentHP <= 0 || maxHP <= 0)
        return;

    float stringXStart = (stringSpacing * 2.0f) / maxHP;

    for (int i = 0; i < currentHP; ++i)
    {
        auto* stringImg = base->AddChild<UIImage>();

        stringImg->SetName("TalismanString_" + std::to_string(i));
        stringImg->SetTexturePath(talisman->GetStringTexture());
        stringImg->SetSize({ stringSize, stringSize });

        float offsetX = -stringXStart + (stringSpacing * i + stringXOffset);
        float offsetY = stringYOffset;

        stringImg->SetPosition({ offsetX, offsetY });
    }
}

void EquipScreen::BuildAvailableList() {
    auto talismans = inventoryData->GetAvailableTalismans();

    availableContainer->SetCount((int)talismans.size());
    availableContainer->RegenerateChildren();

    auto children = availableContainer->GetChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        Talisman* talisman = talismans[i];
        UIElement* child = children[i];

        BuildTalismanVisual(child, talisman);

        if (child->SupportsClick()) {
            child->SetOnClick([this, talisman]() {
                EquipToFirstOpenSlot(talisman);
                });
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

    for (int i = 0; i < slots; ++i) {
        UIElement* child(children[i]);

        if (equipped[i]) {
            BuildTalismanVisual(child, equipped[i]);

            if (child->SupportsClick()) {
                const int slotIndex = i;
                child->SetOnClick([this, slotIndex]() {
                    inventoryData->UnequipTalisman(slotIndex);

                    // Refresh UI
                    BuildAvailableList();
                    BuildEquippedSlots();
                    });
            }
        }
        else {
            // Maybe disable click here
            if (child->SupportsClick()) {
                //child.ClearOnClick();
            }
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

            // Refresh UI
            BuildAvailableList();
            BuildEquippedSlots();
            return;
        }
    }
}