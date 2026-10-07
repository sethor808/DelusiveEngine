#pragma once
#include <string>
#include <memory>
#include <vector>

struct DelusiveInstance;
class Agent;
class SceneSystem;
class Component;
class UIElement;
class UICanvas;
class Talisman;
class BehaviourScript;
class UIScript;

//Maps a serialized type string to a live instance.
//Specialized per owned entity type so DelusiveObject<T> can construct generically.
//Deliberately free of ImGui - the editor's type picker lives in PropertyDraw.
template<typename T>
struct DelusiveFactory;

//Agents and systems come from one table each - a new type is one line there, and the
//editor's menus list it through Types() without being touched
template<>
struct DelusiveFactory<Agent> {
    static std::unique_ptr<Agent> Create(const std::string& type, DelusiveInstance&);
    static std::vector<std::string> Types();
};

template<>
struct DelusiveFactory<SceneSystem> {
    static std::unique_ptr<SceneSystem> Create(const std::string& type, DelusiveInstance&);
    static std::vector<std::string> Types();
};

template<>
struct DelusiveFactory<Component> {
    static std::unique_ptr<Component> Create(const std::string& type, DelusiveInstance&);
};

template<>
struct DelusiveFactory<Talisman> {
    static std::unique_ptr<Talisman> Create(const std::string& type, DelusiveInstance&);
};

template<>
struct DelusiveFactory<UICanvas> {
    //Canvases have a single type - the recipe's type string is ignored
    static std::unique_ptr<UICanvas> Create(const std::string& type, DelusiveInstance&);
};

template<>
struct DelusiveFactory<UIElement> {
    static std::unique_ptr<UIElement> Create(const std::string& type, DelusiveInstance&);
};

template<>
struct DelusiveFactory<BehaviourScript> {
    static std::unique_ptr<BehaviourScript> Create(const std::string& type, DelusiveInstance&);
};

template<>
struct DelusiveFactory<UIScript> {
    static std::unique_ptr<UIScript> Create(const std::string& type, DelusiveInstance&);
};
