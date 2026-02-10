#include <DelusiveExternal/DelusiveScriptPlayer.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <DelusiveExternal/DelusiveInventoryData.h>

DelusiveInventoryData DelusiveScriptPlayer::GetInventory() {
    DelusiveInventoryData data;
    if (IsValid()){
        data.Link(player->GetInventory());
    }
    return data;
}