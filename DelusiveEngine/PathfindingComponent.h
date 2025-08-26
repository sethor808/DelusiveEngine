#pragma once
#include "Component.h"

class PathfindingComponent : public Component {
public:
	PathfindingComponent();

	//Virtual function overrides
	std::unique_ptr<Component> Clone() const override;
	void DrawImGui() override;
	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override {};

	const char* GetType() const override {
		return "PathfinderComponent";
	}

	//void Serialize(std::ofstream& out) const override;
	//void Deserialize(std::ifstream& in) override;

	//Pathfinding logic
	void SetPath(const std::vector<glm::vec2>& newPath);
	void RequestPath(glm::vec2, glm::vec2);
	void GetNextTarget() const;
	void AdvanceIfClose(glm::vec2 position, float threshold = 0.1f);
	void PathComplete() const;
	

private:
	std::vector<glm::vec2> currentPath;
	float speed = 5.0f;
	int currentWaypoint = 0; //TODO: Implement waypoints

	glm::vec4 debugColor = glm::vec4(1, 1, 0, 1);
};