#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Shader.h"
#include "DelusiveUtils.h"
#include "Texture.h"
#include "Renderer.h"

class UIElement {
public:
	UIElement() = default;

	UIElement(const std::string& name, const glm::vec2& position)
		: name(name), position(position) {
	}

	virtual std::unique_ptr<UIElement> Clone() const = 0;

	virtual ~UIElement() = default;

	virtual void Update(float deltaTime) {
		for (auto& child : children) {
			child->Update(deltaTime);
		}
	}

	virtual void Draw(const glm::mat4& proj) {
		for (auto& child : children) {
			child->Draw(proj);
		}
	}

	virtual void HandleMouse(const glm::vec2& mousePos, bool mouseDown) {
		for (auto& child : children) {
			child->HandleMouse(mousePos, mouseDown);
		}
	}

	virtual void HandleInput(const PlayerInputState& input) {
		for (auto& child : children) {
			child->HandleInput(input);
		}
	}
	
	template<typename T, typename... Args>
	T* AddChild(Args&&... args) {
		auto child = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = child.get();
		children.push_back(std::move(child));
		return ptr;
	}

	void AddChild(std::unique_ptr<UIElement> element) {
		children.push_back(std::move(element));
	}

	virtual const std::string& GetTypeName() const = 0;
	
	void SetName(const std::string& _name) { name = _name; }
	void SetEnabled(bool _enabled) { enabled = _enabled; }
	void SetSize(const glm::vec2& _size) { size = _size; }
	const std::string& GetName() const { return name; }
	const glm::vec2& GetPosition() const { return position; }
	void SetPosition(const glm::vec2& pos) { position = pos; }
	
	virtual void DrawImGui() {}
	virtual void Serialize(std::ostream& out) const = 0;
	virtual void Deserialize(std::istream& in) = 0;
protected:
	std::string name;
	bool enabled = true;
	glm::vec2 position;
	glm::vec2 size;
	std::vector<std::unique_ptr<UIElement>> children;
};