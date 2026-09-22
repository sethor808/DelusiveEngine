#include <Delusive/Runtime/Scene/SceneSystem.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <memory>

SceneSystem::SceneSystem(DelusiveInstance& instance)
	: instance(instance), registry(std::make_unique<PropertyRegistry>())
{
    //Derived constructors call RegisterProperties - calling it here would
    //dispatch to the base GetType(), which is pure virtual during base construction
}

SceneSystem::SceneSystem(DelusiveInstance& instance, Scene* _scene)
    : SceneSystem(instance)
{
    this->scene = _scene;
}

SceneSystem::~SceneSystem() = default;

void SceneSystem::RegisterProperties() {
    registry->category = "System";
    registry->type = GetType();

    registry->Register("id", &id);
    registry->Register("name", &name);
}

void SceneSystem::Serialize(DelusiveParser::DataBlock& out) const {
    registry->Serialize(out);
}

void SceneSystem::Deserialize(DelusiveParser::DataBlock& in) {
    registry->Deserialize(in);
    //Owned objects pull their recipes from the library, which is already populated
    registry->Resolve(instance);
}

PlayerAgent* SceneSystem::FetchPlayer() const {
    return scene ? scene->FetchPlayer() : nullptr;
}