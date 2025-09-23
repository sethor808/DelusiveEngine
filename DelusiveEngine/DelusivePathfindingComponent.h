#pragma once
#include <cstdint>
#include "DelusiveComponent.h"
#include <Delusive/Transform.h>

class PathfindingComponent;

class DelusivePathfindingComponent : DelusiveComponent {
public:
	DelusivePathfindingComponent(PathfindingComponent* comp) : comp(comp) {};

	std::string GetType() const override { return "PathfindingComponent"; }

	void Update(float deltaTime);

	//Engine side links
	void SetDestination();
	void RequestPath(glm::vec2, glm::vec2);
	bool HasPath() const;
	bool PathComplete();
	glm::vec2 GetNextPoint();

	float GetSpeed() const;
	void SetSpeed(float);
	std::vector<glm::vec2>& GetCurrentPath();

private:
	PathfindingComponent* comp; //Shallow, no ownership
};