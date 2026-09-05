#include <Delusive/Runtime/Scene/SceneSystem.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <memory>

SceneSystem::SceneSystem(DelusiveInstance& instance)
	: instance(instance), registry(std::make_unique<PropertyRegistry>())
{
    RegisterProperties();
}

SceneSystem::SceneSystem(DelusiveInstance& instance, Scene* _scene)
    : SceneSystem(instance)
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