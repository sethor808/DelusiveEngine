#pragma once
#include <DelusiveExternal/UIScript.h>

class EquipScreen : public UIScript {
    void OnInit() override {};
    void OnUpdate(float) override {};
    void OnDraw() override {};
    void OnEvent() override {};
    void OnClick(UIElement* clicked) override {};

    std::string GetType() const override;
};