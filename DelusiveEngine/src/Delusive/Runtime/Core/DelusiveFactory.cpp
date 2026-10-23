#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>

//Complete types are required here - unique_ptr destruction needs them
#include <Delusive/Runtime/Agents/DelusiveAgents.h>
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Talismans/BasicTalisman.h>
#include <Delusive/Runtime/Talismans/SpeedTalisman.h>
#include <Delusive/Runtime/Talismans/MajorSpeedTalisman.h>
#include <Delusive/Runtime/Scene/PathfindingSystem.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Scripting/UIScript.h>

namespace {
    template<typename T>
    using Maker = std::unique_ptr<T>(*)(DelusiveInstance&);

    template<typename Base, typename Concrete>
    std::unique_ptr<Base> Make(DelusiveInstance& instance) {
        return std::make_unique<Concrete>(instance);
    }

    //Type name as saved in files, and how to make one
    const std::vector<std::pair<std::string, Maker<Agent>>> AgentTypes = {
        { "PlayerAgent",      &Make<Agent, PlayerAgent> },
        { "CameraAgent",      &Make<Agent, CameraAgent> },
        { "EnemyAgent",       &Make<Agent, EnemyAgent> },
        { "EnvironmentAgent", &Make<Agent, EnvironmentAgent> },
    };

    const std::vector<std::pair<std::string, Maker<SceneSystem>>> SystemTypes = {
        { "UIManager",         &Make<SceneSystem, UIManager> },
        { "PathfindingSystem", &Make<SceneSystem, PathfindingSystem> },
    };

    template<typename T>
    std::unique_ptr<T> CreateFrom(const std::vector<std::pair<std::string, Maker<T>>>& table, const std::string& type, DelusiveInstance& instance) {
        for (const auto& [name, make] : table) {
            if (name == type) return make(instance);
        }
        return nullptr;
    }

    template<typename T>
    std::vector<std::string> NamesOf(const std::vector<std::pair<std::string, Maker<T>>>& table) {
        std::vector<std::string> names;
        for (const auto& entry : table) names.push_back(entry.first);
        return names;
    }
}

std::unique_ptr<Agent> DelusiveFactory<Agent>::Create(const std::string& type, DelusiveInstance& instance) {
    return CreateFrom(AgentTypes, type, instance);
}

std::vector<std::string> DelusiveFactory<Agent>::Types() {
    return NamesOf(AgentTypes);
}

std::unique_ptr<SceneSystem> DelusiveFactory<SceneSystem>::Create(const std::string& type, DelusiveInstance& instance) {
    return CreateFrom(SystemTypes, type, instance);
}

std::vector<std::string> DelusiveFactory<SceneSystem>::Types() {
    return NamesOf(SystemTypes);
}

std::unique_ptr<Component> DelusiveFactory<Component>::Create(const std::string& type, DelusiveInstance& instance) {
    return DelusiveComponentFactory::CreateComponentByType(type, instance);
}

std::unique_ptr<Talisman> DelusiveFactory<Talisman>::Create(const std::string& type, DelusiveInstance&) {
    if (type == "BasicTalisman")      return std::make_unique<BasicTalisman>();
    if (type == "SpeedTalisman")      return std::make_unique<SpeedTalisman>();
    if (type == "MajorSpeedTalisman") return std::make_unique<MajorSpeedTalisman>();
    return nullptr;
}

std::unique_ptr<UICanvas> DelusiveFactory<UICanvas>::Create(const std::string&, DelusiveInstance& instance) {
    return std::make_unique<UICanvas>(instance);
}

std::unique_ptr<UIElement> DelusiveFactory<UIElement>::Create(const std::string& type, DelusiveInstance& instance) {
    return DelusiveUI::CreateUIElementByType(type, instance);
}

std::unique_ptr<BehaviourScript> DelusiveFactory<BehaviourScript>::Create(const std::string& type, DelusiveInstance& instance) {
    return instance.scriptManager.CreateEnemyLogicScript(type);
}

std::unique_ptr<UIScript> DelusiveFactory<UIScript>::Create(const std::string& type, DelusiveInstance& instance) {
    return instance.scriptManager.CreateUIScript(type);
}
