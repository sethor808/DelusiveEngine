#pragma once
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <unordered_map>
#include <memory>
#include <string>

class UIManager;

class DelusiveUIRegistry {
public:
	DelusiveUIRegistry(const DelusiveUIRegistry&) = delete;
	DelusiveUIRegistry() = delete;
	DelusiveUIRegistry(DelusiveRenderer&);
	~DelusiveUIRegistry() { SaveAll(); }

    void LinkManager(UIManager* manager) { owner = manager; }
    UIManager* GetManager() { return owner; }

	void LoadAll() { LoadFromFile(registryFile); }
	void SaveAll() const {SaveToFile(registryFile);}

	void LoadFromFile(const std::string&);
	void SaveToFile(const std::string&) const;

	UICanvas* Get(const std::string&) const;
	std::unordered_map<std::string, UICanvas*> GetAll() const;
	bool Exists(const std::string&) const;

	void Register(std::unique_ptr<UICanvas> canvas);
	std::vector<std::string> GetAllNames() const;

private:
	DelusiveRenderer& renderer;
    UIManager* owner;
	inline static const std::string registryFile = CANVAS_DATA;
	std::unordered_map<std::string, std::unique_ptr<UICanvas>> canvases;
};