#pragma once
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>

class UIScript;

class UIScriptContainer : public UIElement {
public:
    UIScriptContainer() = delete;
    UIScriptContainer(const UIScriptContainer&) = delete;
    UIScriptContainer& operator=(const UIScriptContainer&) = delete;
    UIScriptContainer(UIScriptContainer&&) noexcept = default;
    UIScriptContainer& operator=(UIScriptContainer&&) noexcept = default;

    UIScriptContainer(DelusiveInstance&);

    void LinkCanvas(UICanvas* canvas) override;

    void RegisterProperties() override;
    void SetScript(std::unique_ptr<UIScript>);
    UIScript* GetScript() const { return script.get(); }
    void Update(float) override;

	std::unique_ptr<UIElement> Clone() const;
	const std::string GetType() const { return "UIScriptContainer"; }
    void DrawImGui() override;
private:
    DelusiveObject<UIScript> script;
};
