#pragma once
#include <Delusive/DelusiveScriptAgent.h>
#include <Delusive/BehaviourScript.h>
#include <memory>

class BasicFollow : public BehaviourScript {
public:
    BasicFollow(DelusiveScriptAgent* owner);
    void Update(float deltaTime) override;
    std::unique_ptr<BehaviourScript> Clone() const override;

    void SetOwner(DelusiveScriptAgent* scriptAgent) { owner = scriptAgent; }
    void SetTarget(DelusiveScriptAgent* targetAgent) { target = targetAgent; }

private:
    float followDistance = 1.0f;
};