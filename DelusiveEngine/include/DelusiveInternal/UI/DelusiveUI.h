#pragma once
#include <string>
#include <memory>

class UIElement;
class DelusiveRenderer;

namespace DelusiveUI {
	std::string DrawUIElementAddMenu();
	std::unique_ptr<UIElement> CreateUIElementByType(const std::string&, DelusiveRenderer&);
}