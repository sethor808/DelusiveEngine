#pragma once
#include <Delusive/Runtime/Utils/UUID.h>
#include <memory>

template <typename T>
struct DelusiveObject {
    std::unique_ptr<T> object = nullptr;

    DelusiveObject& operator=(std::unique_ptr<T> newObject) {
        object = std::move(newObject);
        return *this;
    }

    T* get() { return object.get(); }
    T* get() const { return object.get(); }
    T* operator->() const { return object.get(); }
    explicit operator bool() const { return object != nullptr; }
};

template <typename T>
struct DelusiveLink {
    UUID id;
    T* cached = nullptr;

    bool dirty = true;

    DelusiveLink& operator=(T* newObject) {
        set(newObject);
        return *this;
    }

    void set(T* toLink) {
        cached = toLink;
        id = toLink ? toLink->GetID() : UUID{};
    }

    T* get() const { return cached; }
    UUID getID() const { return id; }
    T* operator->() const { return cached; }
    explicit operator bool() const { return cached != nullptr; }
};

class UICanvas;
struct DelusiveUILink {
    DelusiveUILink() = default;
    DelusiveUILink(UICanvas* link) { canvasLink = link; }

    UUID id;
    UICanvas* canvasLink = nullptr;
    bool dirty = true;
};