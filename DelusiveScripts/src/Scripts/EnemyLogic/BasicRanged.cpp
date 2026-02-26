#include <Scripts/EnemyLogic/BasicRanged.h>
#include <Delusive/Runtime/Core/Transform.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <iostream>
#include <glm/glm.hpp>

BasicRanged::BasicRanged(Agent* owner) : BasicFollow(owner) { // Call base class constructor
    this->owner = owner;
}

void BasicRanged::Update(float deltaTime) {
    if (!owner || !target) return;
    glm::vec2 ownerPos = owner->GetTransform().position;
    glm::vec2 targetPos = target->GetTransform().position;
    glm::vec2 direction = targetPos - ownerPos;
    
    float distance = glm::length(direction);

    if (currentState == Attacking) {
        // Attack logic can be added here
        return; // Skip movement when attacking
    }

    if (currentState != bufferState) { // If Currently transitioning between states
        transitionTimer += deltaTime; // Increment timer
    }
    if (attackTimer < attackCooldown) {
        attackTimer += deltaTime; // Increment attack timer when not attacking
    }

    
#pragma region Determine Buffered State

    //TODO: Find a way to implement this without writing over bufferState each frame
    if (distance < minDistance) { // Too close to target

        bufferState = Retreating; // Buffer retreating state
        
    }
    else if (distance > maxDistance) { // Too far from target
        bufferState = Chasing; // Buffer chasing state
        
    }
    else if (attackTimer >= attackCooldown){ //TODO: Add logic to determine whether attack should start or orbit
        bufferState = Attacking; // Buffer attacking state
    }
    else
    {
        bufferState = Orbiting; // Buffer orbiting state
    }

#pragma endregion

#pragma region Handle State Transition
    if (bufferState == Attacking) { //TODO: Add Attack logic
        currentState = Attacking; // Immediate transition to attacking
        isTransitioning = false; // Reset transitioning flag
        transitionTimer = 0.0f; // Reset timer
        attackTimer = 0.0f; // Reset attack timer
        return; // Skip movement when attacking
    }
    else {
        attackTimer += deltaTime; // Increment attack timer when not attacking
    }
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
            std::cout << "[Enemy] Retreating" << std::endl;
            glm::vec2 moveDir = -direction / distance; // Move away from target
            owner->GetTransform().position += moveDir * movementSpeed * deltaTime; // Move away
        }
        break;

    case Chasing:
        std::cout << "[Enemy] Chasing" << std::endl;
        if (distance > 0.0001f) { // avoid normalize(0)
            glm::vec2 moveDir = direction / distance; // Move towards target
            owner->GetTransform().position += moveDir * movementSpeed * deltaTime;
        }
        break;
    case Orbiting:
        std::cout << "[Enemy] Orbiting" << std::endl;
        int angle = rand() % 360; // Random angle for orbiting
        glm::vec2 orbitPoint = targetPos + glm::vec2(cos(glm::radians((float)angle)), sin(glm::radians((float)angle))) * orbitRadius; // Calculate orbit point
        owner->GetTransform().position += (orbitPoint - ownerPos) * movementSpeed * deltaTime; // Move towards orbit point
        break;

    }


#pragma endregion

}



std::unique_ptr<BehaviourScript> BasicRanged::Clone() const {  // unique_ptr Clone method
    std::unique_ptr<BehaviourScript> copy = std::make_unique<BasicRanged>();
    
    //return std::make_unique<BasicRanged>(*this);
    
    //Do basic copy - INCLUDE IN ALL COPY CALLS UNLESS SPECIFICALLY NEEDED
    copy->CopyCore(this);

    return copy;
}