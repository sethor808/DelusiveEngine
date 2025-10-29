#pragma once
#include "DelusiveComponent.h"
#include <string>
#include <memory>
#include <vector>

class AnimatorComponent;

class DelusiveAnimatorComponent : DelusiveComponent
{
public:
	DelusiveAnimatorComponent(AnimatorComponent* animator);

	std::string GetType() const override { return "AnimatorComponent"; }
	
	void PlayBranch(const std::string&);
	void Update(float deltaTime);
	void Stop();
	bool IsPlaying();

	int GetCurrentFrame();
	float GetTimeAccumulated();
	std::vector<std::string> GetFlags();

private:
	AnimatorComponent* comp;
	std::string currentBranch;
	std::vector<std::string> flags;
};