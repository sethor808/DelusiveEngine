#pragma once
#include "UICanvas.h"
#include <unordered_map>
#include <memory>
#include <string>

class DelusiveUIRegistry {
public:
	DelusiveUIRegistry(const DelusiveUIRegistry&) = delete;
	DelusiveUIRegistry() = delete;
	DelusiveUIRegistry(DelusiveRenderer&);
	~DelusiveUIRegistry() { SaveAll(); }

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
	inline static const std::string registryFile = CANVAS_DATA;
	std::unordered_map<std::string, std::unique_ptr<UICanvas>> canvases;
};