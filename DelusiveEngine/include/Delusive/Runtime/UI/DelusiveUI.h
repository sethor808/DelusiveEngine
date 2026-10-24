#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

class UIElement;
class UICanvas;
class DelusiveRenderer;
class ScriptManager;

namespace DelusiveUI {
	std::string DrawUIElementAddMenu();
	std::unique_ptr<UIElement> CreateUIElementByType(const std::string&, DelusiveInstance&);
}
