#pragma once
#include <DelusiveInternal/UI/UIElement.h>

class UIScript;

class UIScriptContainer : public UIElement {
public:
    UIScriptContainer() = delete;
    UIScriptContainer(const UIScriptContainer&) = delete;
    UIScriptContainer& operator=(const UIScriptContainer&) = delete;
    UIScriptContainer(UIScriptContainer&&) noexcept = default;
    UIScriptContainer& operator=(UIScriptContainer&&) noexcept = default;
    UIScriptContainer(DelusiveRenderer&);

    void SetScript(std::unique_ptr<UIScript>);
    UIScript* GetScript() const { return script.get(); }
    void Update(float) override;

    //For scripts
    void SetBinding(const std::string& name, const UUID& id);
    bool HasBinding(const std::string& name) const;
    UUID GetBinding(const std::string& name) const;

	std::unique_ptr<UIElement> Clone() const;
	const std::string GetType() const { return "UIScriptContainer"; }
    void DrawImGui() override;

private:
    std::unordered_map<std::string, UUID> bindings;
    std::unique_ptr<UIScript> script;
};
