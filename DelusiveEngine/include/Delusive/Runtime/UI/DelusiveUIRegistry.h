#pragma once
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

class UIManager;

//Live canvases built from the library's .canvas recipes, keyed by UUID
class DelusiveUIRegistry {
public:
	DelusiveUIRegistry(const DelusiveUIRegistry&) = delete;
	DelusiveUIRegistry() = delete;
	DelusiveUIRegistry(DelusiveInstance&);
    ~DelusiveUIRegistry() = default;

    void LinkManager(UIManager* manager) { owner = manager; }
    UIManager* GetManager() { return owner; }

    //Rebuilds every canvas the library has indexed. Replaces what was registered,
    //so any UICanvas* handed out before is invalid afterwards.
	void LoadAll();
	//Writes each canvas back to its own file
	bool SaveAll();

	UICanvas* Get(const UUID&) const;
	//First canvas with this name - names are labels, ids are identity
	UICanvas* Get(const std::string&) const;
	bool Exists(const std::string&) const;
	//Sorted by name for stable editor lists
	std::vector<UICanvas*> List() const;

	void Register(std::unique_ptr<UICanvas> canvas);

private:
    DelusiveInstance& instance;
    UIManager* owner;
	std::unordered_map<UUID, std::unique_ptr<UICanvas>, UUID::Hash> canvases;
};
