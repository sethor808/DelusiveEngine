#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <string>
#include <memory>

class Component;

namespace DelusiveComponentFactory {
    std::string DrawComponentAddMenu();
    std::unique_ptr<Component> CreateComponentByType(const std::string&, DelusiveInstance&);
}