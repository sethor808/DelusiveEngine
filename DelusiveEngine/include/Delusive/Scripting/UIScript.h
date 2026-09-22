#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <Delusive/Runtime/Core/UUID.h>
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
    virtual void RelocateReferences() {}

    virtual void RegisterProperties();
	virtual void Link(UIScriptContainer* root) { rootElement = root; }
    UIScriptContainer* GetRoot() const { return rootElement; }

	//Identity handles
	UUID GetID() const { return id; }
	void SetID(UUID newID) { id = newID; }

	//Block entry points - the registry still does the work, these only expose it
	virtual void Serialize(DelusiveParser::DataBlock& out) const;
	virtual void Deserialize(DelusiveParser::DataBlock& in);

	virtual std::string GetType() const = 0;

    virtual void DrawImGui();
protected:
    UUID id;
    UIScriptContainer* rootElement = nullptr;
    std::unique_ptr<PropertyRegistry> registry;
};