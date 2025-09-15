#include "DelusiveUIRegistry.h"
#include <fstream>
#include <iostream>

DelusiveUIRegistry::DelusiveUIRegistry(DelusiveRenderer& _renderer) 
	: renderer(_renderer)
{

}

void DelusiveUIRegistry::LoadFromFile(const std::string& path) {
	std::ifstream in(path);
	if (!in) return;

	size_t count;
	in >> count;
	in.ignore();

	canvases.clear();
	for (size_t i = 0; i < count; i++) {
		std::string line;

		// Skip the opening [UICanvas] line
		while (std::getline(in, line)) {
			if (line.empty()) continue;
			if (line == "[UICanvas]") break;
		}

		auto canvas = std::make_unique<UICanvas>(renderer);
		canvas->Deserialize(in); // now it starts reading the proper contents
		canvases[canvas->GetName()] = std::move(canvas);
	}
}

void DelusiveUIRegistry::SaveToFile(const std::string& path) const {
	std::ofstream out(path);
	out << canvases.size() << std::endl;
	for (const auto& key : canvases) {
		key.second->Serialize(out);
	}
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