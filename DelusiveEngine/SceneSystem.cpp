#include "SceneSystem.h"
#include "DelusiveRegistry.h"
#include <memory>

SceneSystem::SceneSystem(DelusiveRenderer& _renderer)
	: renderer(_renderer), registry(std::make_unique<PropertyRegistry>())
{
    RegisterProperties();
}

SceneSystem::~SceneSystem() = default;

void SceneSystem::RegisterProperties() {
    registry->Register("name", &name);
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