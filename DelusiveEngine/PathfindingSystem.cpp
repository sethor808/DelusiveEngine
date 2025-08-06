#define GLM_ENABLE_EXPERIMENTAL
#include "Renderer.h"
#include "PathfindingSystem.h"
#include <limits>
#include <algorithm>
#include <cmath>
#include <glm/gtx/hash.hpp>

PathfindingSystem::PathfindingSystem() {
	name = "New PathfindingSystem";
}

std::unique_ptr<SceneSystem> PathfindingSystem::Clone() const {
	auto clone = std::make_unique<PathfindingSystem>();

	// Clone all nodes
	std::unordered_map<const Node*, Node*> originalToClone;
	for (const auto& originalNodePtr : allNodes) {
		if (!originalNodePtr) continue;

		// Copy node
		auto newNode = std::make_unique<Node>(*originalNodePtr);

		// Keep track of mapping from original to cloned node
		originalToClone[originalNodePtr.get()] = newNode.get();

		// Store it in clone->allNodes
		clone->allNodes.push_back(std::move(newNode));
	}

	// Rebuild gridMap with cloned node pointers
	for (const auto& [pos, originalNode] : gridMap) {
		auto it = originalToClone.find(originalNode);
		if (it != originalToClone.end()) {
			clone->gridMap[pos] = it->second;
		}
	}

	return clone;
}

void PathfindingSystem::BuildNavGrid(const std::vector<Node>& nodes) {
	gridMap.clear();
	allNodes.clear();

	for (const Node& n : nodes) {
		auto node = std::make_unique<Node>(n.gridPos, n.worldPos, n.walkable);
		gridMap[n.gridPos] = node.get();
		allNodes.push_back(std::move(node));
	}

	for (const auto& pair : gridMap) {
		Node* node = pair.second;
		glm::ivec2 directions[4] = {
			{1,0}, {-1,0}, {0,1}, {0,-1}
		};

		for (const glm::ivec2& dir : directions) {
			glm::ivec2 neighborPos = node->gridPos + dir;
			auto it = gridMap.find(neighborPos);
			if (it != gridMap.end() && it->second->walkable) {
				node->neighbors.push_back(it->second);
			}
		}
	}
}

Node* PathfindingSystem::GetClosestNode(glm::vec2 worldPos) const {
	Node* closest = nullptr;
	float bestDist = std::numeric_limits<float>::max();

	for (const auto& nodePtr : allNodes) {
		if (!nodePtr->walkable) continue;
		float d = glm::distance(worldPos, nodePtr->worldPos);
		if (d < bestDist) {
			bestDist = d;
			closest = nodePtr.get();
		}
	}

	return closest;
}

float PathfindingSystem::Heuristic(const Node* a, const Node* b) const {
	return glm::distance(a->worldPos, b->worldPos);
}

std::vector<glm::vec2> PathfindingSystem::FindPath(glm::vec2 startWorld, glm::vec2 endWorld) {
	Node* start = GetClosestNode(startWorld);
	Node* goal = GetClosestNode(endWorld);

	if (!start || !goal) return {};

	std::unordered_map<Node*, Node*> cameFrom;
	std::unordered_map<Node*, float> costSoFar;

	struct PQEntry {
		Node* node;
		float priority;
		bool operator>(const PQEntry& other) const {
			return priority > other.priority;
		}
	};

	std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> frontier;
	frontier.push({ start, 0 });

	cameFrom[start] = nullptr;
	costSoFar[start] = 0;

	while (!frontier.empty()) {
		Node* current = frontier.top().node;
		frontier.pop();

		if (current == goal) {
			break;
		}

		for (Node* neighbor : current->neighbors) {
			float newCost = costSoFar[current] + glm::distance(current->worldPos, neighbor->worldPos);
			if (!costSoFar.contains(neighbor) || newCost < costSoFar[neighbor]) {
				costSoFar[neighbor] = newCost;
				float priority = newCost + Heuristic(neighbor, goal);
				frontier.push({ neighbor, priority });
				cameFrom[neighbor] = current;
			}
		}
	}

	std::vector<glm::vec2> path;
	Node* step = goal;

	while (step != nullptr) {
		path.push_back(step->worldPos);
		step = cameFrom[step];
	}

	std::reverse(path.begin(), path.end());
	return path;
}

void PathfindingSystem::DrawDebug(const glm::mat4 projection) const {
	for (const auto& pair : gridMap) {
		const glm::ivec2& gridPos = pair.first;
		const Node* node = pair.second;

		glm::vec2 worldPos = glm::vec2(gridPos);
		glm::vec4 color;

		// Color based on walkability or node state (you can expand this)
		if (node->walkable)
			color = glm::vec4(0.2f, 0.8f, 0.2f, 0.5f); // Green
		else
			color = glm::vec4(0.8f, 0.2f, 0.2f, 0.5f); // Red

		// Draw a unit-size square at node center
		Renderer::DebugDrawRect(worldPos + glm::vec2(0.5f), 1.0f, color);
	}
}