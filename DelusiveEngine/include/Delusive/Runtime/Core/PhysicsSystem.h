#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Agent;

//A collider resolved to world space once per tick. Colliders never rotate, so a box is
//exactly its bounds; circles and lines keep their own data for the exact test.
struct WorldShape {
	ColliderComponent* collider = nullptr;
	Agent* owner = nullptr;
	ColliderType type = ColliderType::Solid;
	ShapeType shape = ShapeType::Box;
	glm::vec2 min{ 0.0f }, max{ 0.0f };	//Bounds - exact for boxes
	glm::vec2 center{ 0.0f };			//Box/circle center, line start
	glm::vec2 end{ 0.0f };				//Line end
	float radius = 0.0f;				//Circle
};

//Owned per scene, so contact state never leaks between the editor and play scenes.
//Each tick: resolve every enabled collider to world space, find every contact against
//that snapshot, then run callbacks and solid separation from the same snapshot.
class PhysicsSystem {
public:
	void Step(const std::vector<std::unique_ptr<Agent>>& agents);

	//Shared with ColliderRenderer so what is drawn is exactly what collides
	static WorldShape BuildShape(const ColliderComponent&);

private:
	struct Contact {
		size_t a, b;
	};

	static bool Interacts(ColliderType, ColliderType);
	static bool Overlaps(const WorldShape&, const WorldShape&);
	void ResolveSolids();

	//Reused every tick so steady state does no allocation
	std::vector<WorldShape> shapes;
	std::vector<Contact> contacts;
};
