#include "DelusiveAnimatorComponent.h"
#include "AnimatorComponent.h"

DelusiveAnimatorComponent::DelusiveAnimatorComponent(AnimatorComponent* animator)
	: comp(animator) {
	flags = comp->GetFlags();
}

void DelusiveAnimatorComponent::PlayBranch(const std::string& branchName) {
	comp->PlayBranch(branchName);
}

void DelusiveAnimatorComponent::Update(float deltaTime) {
	return comp->Update(deltaTime);
}

bool DelusiveAnimatorComponent::IsPlaying() {
	return comp->IsPlaying();
}

int DelusiveAnimatorComponent::GetCurrentFrame() {
	return comp->GetCurrentFrame();
}

float DelusiveAnimatorComponent::GetTimeAccumulated() {
	return comp->GetTimeAccumulated();
}