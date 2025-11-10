#include <DelusiveScripts/EnemyLogic/BasicRanged.h>
#include <DelusiveExternal/Transform.h>
#include <iostream>
#include <glm/glm.hpp>

BasicRanged::BasicRanged(DelusiveScriptAgent* owner) : BasicFollow(owner) { // Call base class constructor
    this->owner = owner;
}

void BasicRanged::Update(float deltaTime) {
    if (!owner || !target) return;
    glm::vec2 ownerPos = owner->transform->position;
    glm::vec2 targetPos = target->transform->position;
    glm::vec2 direction = targetPos - ownerPos;
    
    float distance = glm::length(direction);

    if (currentState == Attacking) {
        // Attack logic can be added here
        return; // Skip movement when attacking
    }

    if (currentState != bufferState) { // If Currently transitioning between states
        transitionTimer += deltaTime; // Increment timer
    }

    
#pragma region Determine Buffered State

    //TODO: Find a way to implement this without writing over bufferState each frame
    if (distance < minDistance) { // Too close to target

        bufferState = Retreating; // Buffer retreating state
        
    }
    else if (distance > maxDistance) { // Too far from target
        bufferState = Chasing; // Buffer chasing state
        
    }
    else { //TODO: Add logic to determine whether attack should start or orbit
        bufferState = Orbiting;
    }

#pragma endregion

#pragma region Handle State Transition

    if (bufferState != currentState) {
        if (!isTransitioning) {
            isTransitioning = true;
            transitionTimer = 0.0f; // Reset timer
        }
        else {
            if (transitionTimer >= transitionBuffer) {
                currentState = bufferState; // Commit to new state
                isTransitioning = false; // Reset transitioning flag
                transitionTimer = 0.0f; // Reset timer
            }
        }
    }
    else {
        if (isTransitioning) { // If we were transitioning but returned to the same state
            isTransitioning = false; // Reset transitioning flag
            transitionTimer = 0.0f; // Reset timer
        }
    }

    

#pragma region Movement Based on State

    switch (currentState) { //movement based on state
    case Retreating:
        if (distance > 0.0001f) { // avoid normalize(0)
            glm::vec2 moveDir = -direction / distance; // Move away from target
            owner->transform->position += moveDir * movementSpeed * deltaTime; // Move away
        }
        break;

    case Chasing:
        if (distance > 0.0001f) { // avoid normalize(0)
            glm::vec2 moveDir = direction / distance; // Move towards target
            owner->transform->position += moveDir * movementSpeed * deltaTime;
        }
        break;
    case Orbiting:
        // Orbiting logic can be added here
        break;

    }

#pragma endregion

}



std::unique_ptr<BehaviourScript> BasicRanged::Clone() const {  // unique_ptr Clone method
    return std::make_unique<BasicRanged>(*this);
}