#include "BasicFollow.h"
#include <Delusive/Transform.h>
#include <iostream>
#include <glm/glm.hpp>

BasicFollow::BasicFollow(DelusiveScriptAgent* owner) {
    this->owner = owner;
}

void BasicFollow::Update(float deltaTime) {
    if (!owner || !target) return;

    glm::vec2 ownerPos = owner->transform->position;
    glm::vec2 targetPos = target->transform->position;

    glm::vec2 direction = targetPos - ownerPos;
    float distance = glm::length(direction);

    if (distance > followDistance) {
        if (distance > 0.0001f) { // avoid normalize(0)
            glm::vec2 moveDir = direction / distance; // safe normalize
            owner->transform->position += moveDir * movementSpeed * deltaTime;
        }
    }
}

std::unique_ptr<BehaviourScript> BasicFollow::Clone() const {
    return std::make_unique<BasicFollow>(*this);
}