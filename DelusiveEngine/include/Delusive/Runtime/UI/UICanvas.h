#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <Delusive/Runtime/Utils/DelusiveUtils.h>
#include <Delusive/Runtime/Scene/UIUUIDManager.h>
#include <Delusive/Runtime/Player/PlayerInputState.h>


class UIManager;
class PropertyRegistry;
class PlayerAgent;
class ScriptManager;

class UICanvas {
public:
	UIUUIDManager idManager;

	UICanvas() = delete;
	UICanvas(const UICanvas&) = delete;
	UICanvas& operator=(const UICanvas&) = delete;
	UICanvas(UICanvas&&) noexcept = default;
	UICanvas& operator=(UICanvas&&) noexcept = default;
	UICanvas(DelusiveInstance&);
	
	~UICanvas();

	void RegisterProperties();

	//Save then load in memory, see DelusiveClone
	std::unique_ptr<UICanvas> Clone() const;

	//Identity handles
	UUID GetID() const { return id; }
	void SetID(UUID newID) { id = newID; }

	//Block entry points - the canvas block lists its top level elements by UUID
	void Serialize(DelusiveParser::DataBlock& out) const;
	void Deserialize(DelusiveParser::DataBlock& in);
	//Emits the canvas plus every element tree it owns, flat
	void CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const;

	//Canvas files hold one canvas. Loading re-reads the file; saving to another
	//canvas's file gives the copy fresh ids (remapped = reload it).
	static std::unique_ptr<UICanvas> FromRecipe(const DelusiveParser::DataBlock&, DelusiveInstance&);
	static std::unique_ptr<UICanvas> LoadFromFile(const std::string& path, DelusiveInstance&);
	bool SaveToFile(const std::string& path, bool* remapped = nullptr);
	//Saves back to the file this canvas came from; a new canvas goes to CANVAS_PATH/<name>
	bool Save(bool* remapped = nullptr);

	void LinkManager(UIManager* manager) { uiManager = manager; }
	void DelinkManager() { uiManager = nullptr; }
    ScriptManager& GetScriptManager() const;
	PlayerAgent* FetchPlayer() const;
	void Update(float);
	void Draw(const glm::mat4&);
	void HandleMouse(const glm::vec2&, bool);
	void HandleInput(const PlayerInputState&);
	void DrawImGui();

	void AddElement(std::unique_ptr<UIElement>);
    UIElement* FindElementByUUID(const UUID& id) { return idManager.Find(id); }
	std::vector<UIElement*> GetElements() const;

	void Reset();
	void SetName(const std::string& _name) { name = _name; }
	const std::string& GetName() const { return name; }
	bool IsActive() const {return active;}
	void SetActive(bool);
private:
	UIManager* uiManager = nullptr;
    DelusiveInstance& instance;
	std::unique_ptr<PropertyRegistry> registry;
	UUID id;
	std::string name;
	bool active = false;

	std::vector<std::unique_ptr<UIElement>> elements;
};