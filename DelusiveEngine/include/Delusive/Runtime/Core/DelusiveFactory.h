#pragma once
#include <string>
#include <memory>

struct DelusiveInstance;
class Component;
class UIElement;
class BehaviourScript;
class UIScript;

//Maps a serialized type string to a live instance.
//Specialized per owned entity type so DelusiveObject<T> can construct generically.
//Deliberately free of ImGui - the editor's type picker lives in PropertyDraw.
template<typename T>
struct DelusiveFactory;

template<>
struct DelusiveFactory<Component> {
    static std::unique_ptr<Component> Create(const std::string& type, DelusiveInstance&);
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
