#include "BasicFollow.h"
#include <glm/glm.hpp>

BasicFollow::BasicFollow(DelusiveScriptAgent* owner)
    : owner(owner) {
}

void BasicFollow::Update(float deltaTime) {
    if (!owner || !target) return;

    glm::vec2 ownerPos = owner->transform.position;
    glm::vec2 targetPos = target->transform.position;

    glm::vec2 direction = targetPos - ownerPos;
    float distance = glm::length(direction);

    if (distance > followDistance) {
        glm::vec2 moveDir = glm::normalize(direction);
        owner->transform.position += moveDir * moveSpeed * deltaTime;
    }
}

std::unique_ptr<BehaviourScript> BasicFollow::Clone() const {
    return std::make_unique<BasicFollow>(*this);
}