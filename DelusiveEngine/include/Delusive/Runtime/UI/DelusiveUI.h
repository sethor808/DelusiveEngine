#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <string>
#include <memory>

class UIElement;
class DelusiveRenderer;
class ScriptManager;

namespace DelusiveUI {
	std::string DrawUIElementAddMenu();
	std::unique_ptr<UIElement> CreateUIElementByType(const std::string&, DelusiveInstance&);
}