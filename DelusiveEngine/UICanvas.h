#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "DelusiveUtils.h"
#include "DelusiveUI.h"

class UIManager;
class PropertyRegistry;
class DelusiveRenderer;

class UICanvas {
public:
	UICanvas() = delete;
	UICanvas(const UICanvas&) = delete;
	UICanvas& operator=(const UICanvas&) = delete;
	UICanvas(UICanvas&&) noexcept = default;
	UICanvas& operator=(UICanvas&&) noexcept = default;
	UICanvas(DelusiveRenderer&);
	
	~UICanvas();

	void RegisterProperties();

	std::unique_ptr<UICanvas> Clone() const;

	void LinkManager(UIManager* manager) { uiManager = manager; }
	void Update(float);
	void Draw(const glm::mat4&);
	void HandleMouse(const glm::vec2&, bool);
	void HandleInput(const PlayerInputState&);
	void DrawImGui();

	void AddElement(std::unique_ptr<UIElement>);

	//Serialize
	void SerializeToFile() const;
	std::unique_ptr<UICanvas> LoadFromFile(const std::string&);
	void Serialize(std::ostream&) const;
	void Deserialize(std::istream&);

	void Reset();
	void SetName(const std::string& _name) { name = _name; }
	const std::string& GetName() const { return name; }
	bool IsActive() const {return active;}
	void SetActive(bool);
private:
	UIManager* uiManager = nullptr;
	DelusiveRenderer& renderer;
	std::unique_ptr<PropertyRegistry> registry;
	std::string name;
	bool active = false;

	std::vector<std::unique_ptr<UIElement>> elements;
};