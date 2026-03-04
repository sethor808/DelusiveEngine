#include <Delusive/Runtime/Scene/SceneSystem.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <memory>

SceneSystem::SceneSystem(DelusiveRenderer& _renderer)
	: renderer(_renderer), registry(std::make_unique<PropertyRegistry>())
{
    RegisterProperties();
}

SceneSystem::SceneSystem(DelusiveRenderer& _renderer, Scene* _scene)
    : SceneSystem(_renderer)
{
    this->scene = _scene;
}

SceneSystem::~SceneSystem() = default;

void SceneSystem::RegisterProperties() {
    registry->Register("name", &name);
}

PlayerAgent* SceneSystem::FetchPlayer() const {
    return scene ? scene->FetchPlayer() : nullptr;
}

void SceneSystem::Serialize(std::ostream& out) const {
    out << "[System " << GetType() << "]\n";
    registry->Serialize(out);
    out << "[/System]\n";
}

void SceneSystem::Deserialize(std::istream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);

        if (line == "[/System]") {
            break; // finished this agent block
        }

        registry->Deserialize(iss);
    }
}