#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>

class Scene;
class PropertyRegistry;
class DelusiveRenderer;
class PlayerAgent;

class SceneSystem {
public:
	SceneSystem() = delete;
	SceneSystem(DelusiveRenderer&);
    SceneSystem(DelusiveRenderer&, Scene*);
	virtual ~SceneSystem();

	virtual void LinkScene(Scene* _scene) { scene = _scene; }
	virtual Scene* GetScene() const { return scene; }
	virtual PlayerAgent* FetchPlayer() const;
    virtual void Init() {}

	virtual void RegisterProperties();

	virtual void Update(float) = 0;
	virtual void Draw(const glm::mat4&) = 0;
	virtual void Reset() = 0;
	virtual void DrawImGui() {}
	virtual void SetEditorMode(bool editor) { editorMode = editor; }

	virtual void SetName(std::string _name) { name = _name; }
	virtual std::string GetName() { return name; }

	virtual std::string GetType() const = 0;

	virtual std::unique_ptr<SceneSystem> Clone() const = 0;

	void SaveToFile(const std::string&) const {};
	virtual void SaveToFile(std::ofstream&) const {};

	virtual void Serialize(std::ostream&) const;
	virtual void Deserialize(std::istream&);
protected:
	Scene* scene = nullptr;
	DelusiveRenderer& renderer;
	std::unique_ptr<PropertyRegistry> registry;
	bool editorMode = false;
	std::string name;
};