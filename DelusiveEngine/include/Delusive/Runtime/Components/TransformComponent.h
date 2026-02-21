#pragma once
#include <Delusive/Runtime/Core/Transform.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

struct TransformComponent : public Transform {
public:
    void RegisterProperties(PropertyRegistry &registry) {
        registry.Register("position", &position);
        registry.Register("scale", &scale);
        registry.Register("rotation", &rotation);
    }
};