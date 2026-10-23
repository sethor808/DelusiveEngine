#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <algorithm>

DelusiveUIRegistry::DelusiveUIRegistry(DelusiveInstance& instance)
    : instance(instance), owner(nullptr)
{

}

void DelusiveUIRegistry::LoadAll() {
	canvases.clear();

	for (const DelusiveParser::DataBlock* block : instance.delusiveLibrary.List("UICanvas")) {
		Register(UICanvas::FromRecipe(*block, instance));
	}
}

bool DelusiveUIRegistry::SaveAll() {
	bool success = true;

	for (auto& [id, canvas] : canvases) {
		if (!canvas->Save()) success = false;
	}

	return success;
}

UICanvas* DelusiveUIRegistry::Get(const UUID& id) const {
	auto canv = canvases.find(id);
	return canv == canvases.end() ? nullptr : canv->second.get();
}

UICanvas* DelusiveUIRegistry::Get(const std::string& name) const {
	for (const auto& [id, canvas] : canvases) {
		if (canvas->GetName() == name) return canvas.get();
	}
	return nullptr;
}

bool DelusiveUIRegistry::Exists(const std::string& name) const {
	return Get(name) != nullptr;
}

std::vector<UICanvas*> DelusiveUIRegistry::List() const {
	std::vector<UICanvas*> list;
	list.reserve(canvases.size());

	for (const auto& [id, canvas] : canvases) {
		list.push_back(canvas.get());
	}

	std::sort(list.begin(), list.end(), [](UICanvas* a, UICanvas* b) { return a->GetName() < b->GetName(); });
	return list;
}

void DelusiveUIRegistry::Register(std::unique_ptr<UICanvas> canvas) {
	if (!canvas) return;
	if (!canvas->GetID().IsValid()) canvas->SetID(UUID::GenerateRandom());

	UUID id = canvas->GetID();
	canvases[id] = std::move(canvas);
}
