#include <DelusiveExternal/DelusiveTalismanData.h>
#include <Delusive/Runtime/Talismans/Talisman.h>

std::string DelusiveTalismanData::GetType() {
    return talisman->GetType();
}

int DelusiveTalismanData::GetMaxHP() {
    return talisman->GetMaxHP();
}

int DelusiveTalismanData::GetCurrentHP() {
    return talisman->GetCurrentHP();
}

bool DelusiveTalismanData::GetIsBroken() {
    return talisman->GetIsBroken();
}

std::string DelusiveTalismanData::ConstantPassiveDesc() {
    return talisman->ConstantPassiveDesc();
}

std::string DelusiveTalismanData::WhileActiveDesc() {
    return talisman->WhileActiveDesc();
}

std::string DelusiveTalismanData::OnConsumeDesc() {
    return talisman->OnConsumeDesc();
}