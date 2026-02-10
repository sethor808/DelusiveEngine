#pragma once
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

class DelusiveUIScriptContainer;

class UIScript
{
public:
	virtual ~UIScript() = default;

	virtual void OnInit() {}
    virtual void OnEnable() {}
	virtual void OnUpdate(float) {}
	virtual void OnDraw() {}
	virtual void OnEvent() {}
	virtual void OnClick(DelusiveUIScriptContainer* clicked) {}

	virtual void Link(DelusiveUIScriptContainer* root) { rootElement = root; }
    DelusiveUIScriptContainer* GetRoot() const { return rootElement; }

	virtual std::string GetType() const = 0;

    //TODO: Make sure de/serialation works later
    virtual void Serialize(std::ostream& os) const {};
    virtual void Deserialize(std::istream& is) {};
protected:
    DelusiveUIScriptContainer* rootElement = nullptr;
};