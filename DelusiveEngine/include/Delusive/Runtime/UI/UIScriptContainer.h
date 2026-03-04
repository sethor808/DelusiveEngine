#pragma once
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>

class UIScript;
struct DelusiveUIScript;

class UIScriptContainer : public UIElement {
public:
    UIScriptContainer() = delete;
    UIScriptContainer(const UIScriptContainer&) = delete;
    UIScriptContainer& operator=(const UIScriptContainer&) = delete;
    UIScriptContainer(UIScriptContainer&&) noexcept = default;
    UIScriptContainer& operator=(UIScriptContainer&&) noexcept = default;

    UIScriptContainer(DelusiveRenderer&, ScriptManager&);

    void SetScript(std::unique_ptr<UIScript>);
    DelusiveUIScript* GetScriptContainer() const { return scriptContainer.get(); }
    void Update(float) override;

	std::unique_ptr<UIElement> Clone() const;
	const std::string GetType() const { return "UIScriptContainer"; }
    void DrawImGui() override;
private:
    ScriptManager& scriptManager;
    std::unique_ptr<DelusiveUIScript> scriptContainer;
    //std::unique_ptr<UIScript> script;
};
