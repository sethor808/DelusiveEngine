#pragma once
#include "Sprite.h"
#include "Animator.h"
#include "TransformData.h"
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <iostream>

class Agent {
public:
	Agent();
	virtual ~Agent();

	virtual void Update(float deltaTime) = 0;
	virtual void Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const = 0;

	void SetPosition(const glm::vec2& pos);
	void SetRotation(const float rotation);
	void SetScale(const glm::vec2 scale);

	TransformComponent& GetTransform();
	const TransformComponent& GetTransform() const;

	void SaveToFile(std::ofstream& out) const;
	void LoadFromFile(std::ifstream& in);

	void SetName(const std::string& newName) { name = newName; }
	const std::string& GetName() const { return name; }
private:
	Collider collider;
	std::string name;

protected:
	TransformComponent transform;
};