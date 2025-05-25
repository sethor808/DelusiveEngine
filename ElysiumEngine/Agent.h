#pragma once
#include "TransformData.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

class Component;
class Collider;

class Agent {
public:
	Agent();
	virtual ~Agent();

	virtual void Update(float deltaTime) = 0;
	virtual void Draw(const glm::mat4& projection) const = 0;

	void SetPosition(const glm::vec2& pos);
	void SetRotation(const float rotation);
	void SetScale(const glm::vec2 scale);

	TransformComponent& GetTransform();
	const TransformComponent& GetTransform() const;

	// Add a component of type T and forward any constructor arguments
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		component->SetOwner(this);
		T* ptr = component.get();
		components.push_back(std::move(component));
		return ptr;
	}

	// Get a component of type T, returns nullptr if not found
	template<typename T>
	T* GetComponent() const {
		for (const auto& c : components) {
			if (auto ptr = dynamic_cast<T*>(c.get())) {
				return ptr;
			}
		}
		return nullptr;
	}

	// Remove the first component of type T
	template<typename T>
	void RemoveComponent() {
		components.erase(
			std::remove_if(
				components.begin(),
				components.end(),
				[](const std::unique_ptr<Component>& c) {
					return dynamic_cast<T*>(c.get()) != nullptr;
				}
			),
			components.end()
		);
	}

	const std::vector<std::unique_ptr<Component>>& GetComponents() const;

	void RemoveComponentByPointer(Component* target);
	void SaveToFile(std::ofstream& out) const;
	void LoadFromFile(std::ifstream& in);

	void SetName(const std::string& newName) { name = newName; }
	const std::string& GetName() const { return name; }
private:
	std::string name;

protected:
	TransformComponent transform;
	std::vector<std::unique_ptr<Component>> components;
};