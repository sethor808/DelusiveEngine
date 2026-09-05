#pragma once
#include <Scripts/EnemyLogic/BasicFollow.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <memory>

class BasicFollow : public BehaviourScript {
public:
    BasicFollow() = default;
    BasicFollow(Agent* owner);

    std::string GetType() override { return "BasicFollow"; }

    void Update(float deltaTime) override;
    std::unique_ptr<BehaviourScript> Clone() const override;
private:
    float followDistance = 1.0f;
};