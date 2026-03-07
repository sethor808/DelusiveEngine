#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <fstream>
#include <iostream>

DelusiveUIRegistry::DelusiveUIRegistry(DelusiveRenderer& _renderer)
    : renderer(_renderer), owner(nullptr)
{

}

void DelusiveUIRegistry::LoadFromFile(const std::string& path) {
	std::ifstream file(path);
    if (!file) {
        std::cerr << "[DelusiveUIRegistry] Error opening canvas data file." << std::endl;
        return;
    }

    auto blocks = DelusiveParser::ParseFile(file);

    canvases.clear();
    for (auto& block : blocks) {
        if (block.category == "UICanvas") {
            auto canvas = std::make_unique<UICanvas>(renderer);
            canvas->LinkManager(owner);
            canvas->Deserialize(block);
            canvases[canvas->GetName()] = std::move(canvas);
        }
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