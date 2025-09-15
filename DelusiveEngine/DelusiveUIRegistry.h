#pragma once
#include "UICanvas.h"
#include <unordered_map>
#include <memory>
#include <string>

class DelusiveUIRegistry {
public:
	static DelusiveUIRegistry& Instance();
	~DelusiveUIRegistry() { SaveAll(); }

	void LoadAll() { LoadFromFile(registryFile); }
	void SaveAll() const {SaveToFile(registryFile);}

	void LoadFromFile(const std::string&);
	void SaveToFile(const std::string&) const;

	UICanvas* Get(const std::string&);
	std::unordered_map<std::string, UICanvas*> GetAll() const;
	bool Exists(const std::string&) const;

	void Register(std::unique_ptr<UICanvas> canvas);
	std::vector<std::string> GetAllNames() const;

private:
	DelusiveUIRegistry() = default;
	bool init = false;
	inline static const std::string registryFile = "assets/canvasData/ui_canvases.txt";
	std::unordered_map<std::string, std::unique_ptr<UICanvas>> canvases;
};