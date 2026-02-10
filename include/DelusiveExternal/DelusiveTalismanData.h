#pragma once
#include <string>

class Talisman;

class DelusiveTalismanData {
public:
    ~DelusiveTalismanData() = default;

    bool IsValid() const { return talisman != nullptr; }
    void Link(Talisman* talisman) { this->talisman = talisman; }
    Talisman* GetLink() { return talisman; }

    std::string GetType();

    int GetMaxHP();
    int GetCurrentHP();
    //Currently disabled - don't see a reason a script would need this right now
    //bool TakeDamage();

    bool GetIsBroken();
    //Currently disabled - don't see a reason a script would need this right now
    //void SetIsBroken(bool broken) { isBroken = broken; }

    //Currently disabled - don't see a reason a script would need this right now
    //void ConstantPassive();
    //void WhileActive();
    //void OnConsume();
    
    std::string ConstantPassiveDesc();
    std::string WhileActiveDesc();
    std::string OnConsumeDesc();

    std::string GetBaseTexture() const;
    std::string GetGlyphTexture() const;
    std::string GetStringTexture() const;
private:
    Talisman* talisman = nullptr; //non-owning
};