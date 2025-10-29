#pragma once
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

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

	virtual void Link(UIElement* root) { rootElement = root; }
	UIElement* GetRoot() const { return rootElement; }

	virtual std::string GetType() const = 0;

	virtual void Serialize(std::ostream& os) const;
	virtual void Deserialize(std::istream& is);
private:
	UIElement* rootElement = nullptr;
};