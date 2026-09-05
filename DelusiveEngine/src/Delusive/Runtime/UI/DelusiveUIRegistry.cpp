#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <fstream>
#include <iostream>

DelusiveUIRegistry::DelusiveUIRegistry(DelusiveInstance& instance)
    : instance(instance), owner(nullptr)
{

}

UICanvas* DelusiveUIRegistry::Get(const std::string& name) const{
	auto canv = canvases.find(name);
	if (canv != canvases.end()) {
		return canv->second.get();
	}
	return nullptr;
}

std::unordered_map<std::string, UICanvas*> DelusiveUIRegistry::GetAll() const {
	std::unordered_map<std::string, UICanvas*> allCanvases;
	for (const auto& key : canvases) {
		allCanvases[key.first] = key.second.get();
	}
	return allCanvases;
}

std::vector<std::string> DelusiveUIRegistry::GetAllNames() const {
	std::vector<std::string> names;
	names.reserve(canvases.size()); // efficiency
	for (const auto& [name, _] : canvases) {
		names.push_back(name);
	}
	return names;
}

bool DelusiveUIRegistry::Exists(const std::string& name) const {
	return canvases.contains(name);
}

void DelusiveUIRegistry::Register(std::unique_ptr<UICanvas> canvas) {
	std::string name = canvas->GetName();
	canvases[name] = std::move(canvas);
}