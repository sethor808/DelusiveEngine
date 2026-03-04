#pragma once
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

class UIScriptContainer;
class PropertyRegistry;

class UIScript
{
public:
    UIScript();
    virtual ~UIScript();

	virtual void OnInit() {}
    virtual void OnEnable() {}
	virtual void OnUpdate(float) {}
	virtual void OnDraw() {}
	virtual void OnEvent() {}
	virtual void OnClick(UIScriptContainer* clicked) {}

    virtual void RegisterProperties() {}
	virtual void Link(UIScriptContainer* root) { rootElement = root; }
    UIScriptContainer* GetRoot() const { return rootElement; }

	virtual std::string GetType() const = 0;

    //TODO: Make sure de/serialation works later
    virtual void Serialize(std::ostream&) const;
    virtual void Deserialize(std::istream&);
    virtual void DrawImGui();
protected:
    UIScriptContainer* rootElement = nullptr;
    std::unique_ptr<PropertyRegistry> registry;
};