#include <DelusiveExternal/DelusiveUIButton.h>
#include <Delusive/Runtime/UI/UIButton.h>

void DelusiveUIButton::SetLabel(std::string& label) {
    if (auto* c = dynamic_cast<UIButton*>(element)) {
        c->SetLabel(label);
    }
}

void DelusiveUIButton::SetOnClick(std::function<void()> callback) {
    if (auto* c = dynamic_cast<UIButton*>(element)) {
        c->SetOnClick(callback);
    }
}