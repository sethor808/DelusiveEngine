#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "DelusiveUtils.h"
#include "DelusiveUI.h"
#include "DelusiveRegistry.h"
#include "DelusiveRenderer.h"

class UICanvas {
public:
	UICanvas() = delete;
	UICanvas(DelusiveRenderer& _renderer)
		: renderer(_renderer) {
		RegisterProperties();
	}
	
	void RegisterProperties();

	std::unique_ptr<UICanvas> Clone() const;

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
	const std::string& GetFilePath() const { return filePath; }
	bool IsActive() const {return active;}
	void SetActive(bool);
private:
	DelusiveRenderer& renderer;
	PropertyRegistry registry;
	std::string name;
	std::string filePath;
	bool active = false;

	std::vector<std::unique_ptr<UIElement>> elements;
};