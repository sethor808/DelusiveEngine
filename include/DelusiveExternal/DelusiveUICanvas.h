// DelusiveExternal/DelusiveUICanvas.h
#pragma once
#include <string>
#include <DelusiveExternal/UUID.h>
#include <DelusiveExternal/ScriptUIElement.h>

class UICanvas;
class DelusiveScriptPlayer;
class ScriptUIElement;

class DelusiveUICanvas {
public:
    DelusiveUICanvas() = default;
    
    void Link(UICanvas* canvas) { this->canvas = canvas; }

    bool IsValid() const { return canvas != nullptr; }

    // Identity / state
    std::string GetName() const;
    bool IsActive() const;
    void SetActive(bool);
        
    // Scene bridge
    DelusiveScriptPlayer GetPlayer() const;

    // UI lookup
    ScriptUIElement FindElementByUUID(const UUID& id) const;

private:
    UICanvas* canvas = nullptr; // non-owning
};