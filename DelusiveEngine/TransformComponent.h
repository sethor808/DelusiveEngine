#pragma once
#include <Delusive/Transform.h>
#include "DelusiveRegistry.h"

struct TransformComponent : public Transform {
public:
    void RegisterProperties(PropertyRegistry &registry) {
        registry.Register("position", &position);
        registry.Register("scale", &scale);
        registry.Register("rotation", &rotation);
    }
};