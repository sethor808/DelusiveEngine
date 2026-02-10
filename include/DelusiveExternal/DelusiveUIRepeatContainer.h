#pragma once
#include <DelusiveExternal/ScriptUIElement.h>

class UIRepeatContainer;

class DelusiveUIRepeatContainer : public ScriptUIElement {
public:
    DelusiveUIRepeatContainer() = default;

    void SetCount(int);
    void SetRows(int);
    void SetSpacing(float);

    void Regenerate();
};