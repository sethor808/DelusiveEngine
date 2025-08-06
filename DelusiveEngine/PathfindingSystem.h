#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "SceneSystem.h"

struct Node {
	glm::ivec2 gridPos;
	glm::vec2 worldPos;
	bool walkable = true;

	std::vector<Node*> neighbors;

	Node(glm::ivec2 grid, glm::vec2 world, bool walk = true)
		: gridPos(grid), worldPos(world), walkable(walk) {};
};

namespace std {
	template <>
	struct hash<glm::ivec2> {
		std::size_t operator()(const glm::ivec2& k) const noexcept {
			return ((std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1)) >> 1);
		}
	};
}

class PathfindingSystem : public SceneSystem {
public:
	PathfindingSystem();
	void BuildNavGrid(const std::vector<Node>&);
	std::vector<glm::vec2> FindPath(glm::vec2, glm::vec2);

	//TODO: Implement later as needed
	void Update(float) override {};
	void Draw(const glm::mat4&) override {};
	void Reset() override {};
	void DrawImGui() override {};

	std::unique_ptr<SceneSystem> Clone() const override;

	void Serialize(std::ostream& out) const override {};
	void Deserialize(std::istream& in) override {};

	void DrawDebug(const glm::mat4 projection) const;

private:
	std::unordered_map<glm::ivec2, Node*> gridMap;
	std::vector<std::unique_ptr<Node>>  allNodes;

	Node* GetClosestNode(glm::vec2) const;
	float Heuristic(const Node* a, const Node* b) const;
};