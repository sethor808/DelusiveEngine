#pragma once
#include "UIElement.h"


class UIScriptContainer : public UIElement {
public:
	UIScriptContainer() = delete;
	UIScriptContainer(const UIScriptContainer&) = delete;
	UIScriptContainer& operator=(const UIScriptContainer&) = delete;
	UIScriptContainer(UIScriptContainer&&) noexcept = default;
	UIScriptContainer& operator=(UIScriptContainer&&) noexcept = default;
	UIScriptContainer(DelusiveRenderer&);

	std::unique_ptr<UIElement> Clone() const;
	const std::string GetType() const { return "UIScriptContainer"; }
};
