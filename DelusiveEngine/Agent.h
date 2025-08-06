#pragma once
#include "TransformData.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <type_traits>
#include <string>
#include <fstream>
#include <SDL3/SDL.h>
#include "DelusiveUtils.h"

class Component;
class Collider;

class Agent {
public:
	TransformComponent transform;

	Agent();
	virtual ~Agent();

	//Mandatory virtual functions
	virtual std::unique_ptr<Agent> Clone() const = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw(const glm::mat4& projection) const = 0;
	virtual void DrawImGui() {};
	
	//Virtual functions
	virtual void OnHit() {};
	void HandleMouse(const glm::vec2&, bool);
	virtual void HandleInput(const PlayerInputState&) {}
	virtual GLuint RenderAgentToTexture(int width = 128, int height = 128);

	void SetPosition(const glm::vec2& pos);
	void SetRotation(const float rotation);
	void SetScale(const glm::vec2 scale);

	TransformComponent& GetTransform();
	const TransformComponent& GetTransform() const;

	template<typename T>
	T* GetComponentOfType() {
		for (auto& comp : components) {
			if (T* casted = dynamic_cast<T*>(comp.get()))
				return casted;
		}
		return nullptr;
	}

	template<typename T>
	std::vector<T*> GetComponentsOfType() {
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

		std::vector<T*> result;

		for (auto& comp : components) {
			if (T* casted = dynamic_cast<T*>(comp.get())) {
				result.push_back(casted);
			}
		}

		return result;
	}

	// Add a component of type T and forward any constructor arguments
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		component->SetOwner(this);
		component->SetID(nextComponentID++);
		T* ptr = component.get();
		components.push_back(std::move(component));
		return ptr;
	}

	void AddRawComponent(std::unique_ptr<Component>);

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

	Component* GetComponentByName(const std::string&);
	Component* GetComponentByID(uint64_t id);
	const std::vector<std::unique_ptr<Component>>& GetComponents() const;
	void RemoveComponentByPointer(Component* target);

	void SaveToFile(const std::string&) const;
	virtual void SaveToFile(std::ofstream& out) const;
	void LoadFromFile(const std::string&);
	virtual void LoadFromFile(std::ifstream& in);

	void SetName(const std::string& newName) { name = newName; }
	const std::string& GetName() const { return name; }	

protected:
	std::vector<std::unique_ptr<Component>> components;
	std::string name;
	uint64_t nextComponentID = 1;
};