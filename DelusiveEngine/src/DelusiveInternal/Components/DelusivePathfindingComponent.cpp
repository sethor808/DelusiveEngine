#include <DelusiveExternal/DelusivePathfindingComponent.h>
#include <DelusiveInternal/Components/PathfindingComponent.h>

void DelusivePathfindingComponent::Update(float deltaTime) {
	comp->Update(deltaTime);
}

void DelusivePathfindingComponent::RequestPath(glm::vec2 a, glm::vec2 b) {
	comp->RequestPath(a, b);
}

bool DelusivePathfindingComponent::PathComplete() {
	return comp->PathComplete();
}

glm::vec2 DelusivePathfindingComponent::GetNextPoint() {
	return comp->GetNextPoint();
}

float DelusivePathfindingComponent::GetSpeed() const {
	return comp->GetSpeed();
}

void DelusivePathfindingComponent::SetSpeed(float speed) {
	comp->SetSpeed(speed);
}

std::vector<glm::vec2>& DelusivePathfindingComponent::GetCurrentPath() {
	return comp->GetCurrentPath();
}