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

//One item of bound game data, fields by name ("hp", "base", ...)
using UIBindItem = std::unordered_map<std::string, std::string>;

namespace DelusiveUI {
	std::string DrawUIElementAddMenu();
	std::unique_ptr<UIElement> CreateUIElementByType(const std::string&, DelusiveInstance&);

	//Data binding - named lists of game data a UIRepeatContainer repeats over
	const std::vector<std::string>& ListBindSources();
	std::vector<UIBindItem> FetchBindSource(const std::string&, UICanvas&);
	//Null when the element has no binding for this field or the item lacks it
	const std::string* FetchBindField(const UIBindItem&, const std::string& field);
}
