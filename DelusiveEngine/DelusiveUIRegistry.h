#pragma once
#include "UICanvas.h"
#include <unordered_map>
#include <memory>
#include <string>

class DelusiveUIRegistry {
public:
	static DelusiveUIRegistry& Instance();

	void LoadFromFile(const std::string&);
	void SaveToFile(const std::string&) const;

	UICanvas* Get(const std::string&);
	std::unordered_map<std::string, UICanvas*> GetAll() const;
	bool Exists(const std::string&) const;

	void Register(std::unique_ptr<UICanvas> canvas);
	std::vector<std::string> GetAllNames() const;

private:
	DelusiveUIRegistry() = default;
	std::unordered_map<std::string, std::unique_ptr<UICanvas>> canvases;
};