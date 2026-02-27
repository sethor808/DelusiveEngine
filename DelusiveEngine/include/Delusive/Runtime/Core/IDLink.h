#pragma once
#include <Delusive/Runtime/Utils/UUID.h>

class Scene;
//UUID Packager
struct DelusiveIDLink {
    DelusiveIDLink() = default;
    DelusiveIDLink(Scene* link) { sceneLink = link; }

    UUID id;
    Scene* sceneLink = nullptr;
    bool dirty = true;
};