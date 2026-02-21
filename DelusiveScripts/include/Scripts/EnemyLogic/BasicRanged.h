#pragma once
#include <Scripts/EnemyLogic/BasicFollow.h>
#include <random>

class BasicRanged : public BasicFollow {
public:
    BasicRanged() = default; // Default constructor
    BasicRanged(Agent* owner); // Constructor with owner
    std::unique_ptr<BehaviourScript> Clone() const override; // unique_ptr Clone method
    void Update(float deltaTime) override; // Update method needs to be different since target position is not the agent, but a point around the agent
    

    enum State { //Different states for the ranged enemy
        Orbiting, //Player has not moved away enough, enemy orbits around player
        Retreating, //Player is too close, enemy moves away from player
        Chasing, //Player is too far, enemy moves towards player
        Attacking //Player is within range, enemy attacks player
    };
    State GetRangedState(); // Method to get the current state of the ranged enemy

    
#pragma region Getters and Setters

    void SetMinDistance(float distance) { minDistance = distance; } // Setter for minimum distance
    void SetMaxDistance(float distance) { maxDistance = distance; } // Setter for maximum distance
    void SetOrbitRadius(float radius) { orbitRadius = radius; } // Setter for orbit radius
    State GetCurrentState() const { return currentState; } // Getter for current state
    float GetMinDistance() const { return minDistance; } // Getter for minimum distance
    float GetMaxDistance() const { return maxDistance; } // Getter for maximum distance
    float GetOrbitRadius() const { return orbitRadius; } // Getter for orbit radius

#pragma endregion

#pragma region Non-Overwritten Methods

    /* NON-MODIFIED INHERITED METHODS
    * void SetOwner(DelusiveScriptAgent* scriptAgent) { owner = scriptAgent; } // Inherited from BasicFollow, no changes needed
    * void SetTarget(DelusiveScriptAgent* targetAgent) { target = targetAgent; } //Inherited from BasicFollow, target setting is the same
    */

#pragma endregion
    


private:



#pragma region Variables

    State currentState = Orbiting;
    State bufferState = Orbiting; // State to buffer against rapid changes
    float minDistance = 3.0f; // Minimum distance to maintain
    float maxDistance = 7.0f; // Maximum distance to maintain
    float orbitRadius = 5.0f; // Radius for orbiting behavior
    float transitionBuffer = 0.5f; // Buffer to prevent rapid state changes
    float transitionTimer = 0.0f; // Timer to track time spent in current state
    float attackCooldown = 2.0f; // Time between attacks
    float attackTimer = 0.0f; // Timer to track time since last attack
    bool isTransitioning = false; // Flag to indicate if currently transitioning between states to avoid rewriting variable each frame

#pragma endregion
    
};