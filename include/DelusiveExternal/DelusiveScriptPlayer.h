#include <DelusiveExternal/DelusiveScriptAgent.h>

class PlayerAgent;
class DelusiveInventoryData;

class DelusiveScriptPlayer : public DelusiveScriptAgent {
public:
    void Link(PlayerAgent* player) { this->player = player; }

    bool IsValid() const;

    DelusiveInventoryData GetInventory();

private:
    PlayerAgent* player = nullptr;
};