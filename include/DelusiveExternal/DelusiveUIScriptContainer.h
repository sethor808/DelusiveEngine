#pragma once
#include <DelusiveExternal/DelusiveUICanvas.h>

class UIScriptContainer;
class UIScript;

class DelusiveUIScriptContainer {
public:
    DelusiveUIScriptContainer() = default;

    void Link(UIScriptContainer* container) { rootContainer = container; }
    UIScriptContainer* GetLink() { return rootContainer; }
    bool IsValid() const { return rootContainer != nullptr; }
    DelusiveUICanvas GetCanvas();

    void SetScript(std::unique_ptr<UIScript>);
    UIScript* GetScript() const;

    //For scripts
    void SetBinding(const std::string&, const UUID&);
    bool HasBinding(const std::string&) const;
    UUID GetBinding(const std::string&) const;
private:
    UIScriptContainer* rootContainer = nullptr;
};