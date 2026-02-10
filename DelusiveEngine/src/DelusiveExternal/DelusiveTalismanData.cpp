#include <Delusive/Runtime/Talismans/Talisman.h>
#include <DelusiveExternal/DelusiveTalismanData.h>

std::string DelusiveTalismanData::GetType() {
    return IsValid() ? talisman->GetType() : "";
}

int DelusiveTalismanData::GetMaxHP() {
    return IsValid() ? talisman->GetMaxHP() : 0;
}

int DelusiveTalismanData::GetCurrentHP() {
    return IsValid() ? talisman->GetCurrentHP() : 0;
}

bool DelusiveTalismanData::GetIsBroken() {
    return IsValid() ? talisman->GetIsBroken() : true;
}

std::string DelusiveTalismanData::ConstantPassiveDesc() {
    return IsValid() ? talisman->ConstantPassiveDesc() : "Cannot retrieve";
}

std::string DelusiveTalismanData::WhileActiveDesc() {
    return IsValid() ? talisman->WhileActiveDesc() : "Cannot retrieve";
}

std::string DelusiveTalismanData::OnConsumeDesc() {
    return IsValid() ? talisman->OnConsumeDesc() : "Cannot retrieve";
}

std::string DelusiveTalismanData::GetBaseTexture() const {
    return IsValid() ? talisman->GetBaseTexture() : "Cannot retrieve";
}

std::string DelusiveTalismanData::GetGlyphTexture() const {
    return IsValid() ? talisman->GetGlyphTexture() : "Cannot retrieve";
}

std::string DelusiveTalismanData::GetStringTexture() const {
    return IsValid() ? talisman->GetStringTexture() : "Cannot retrieve";
}