#pragma once

class UIElement;

class UIScript
{
public:
	virtual ~UIScript() = default;

	virtual void OnInit() {}
	virtual void OnUpdate(float) {}
	virtual void OnDraw() {}
	virtual void OnEvent() {}
	virtual void OnClick(UIElement* clicked) {}

	void Link(UIElement* root) { rootElement = root; }

private:
	UIElement* rootElement = nullptr;
};