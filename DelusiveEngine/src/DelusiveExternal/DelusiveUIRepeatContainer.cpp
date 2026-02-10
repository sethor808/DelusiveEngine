#include <DelusiveExternal/DelusiveUIRepeatContainer.h>
#include <Delusive/Runtime/UI/UIRepeatContainer.h>

void DelusiveUIRepeatContainer::SetCount(int count) {
    if (auto* c = dynamic_cast<UIRepeatContainer*>(element)) {
        c->SetCount(count);
    }
}

void DelusiveUIRepeatContainer::SetRows(int rows) {
    if (auto* c = dynamic_cast<UIRepeatContainer*>(element)) {
        c->SetRows(rows);
    }
}

void DelusiveUIRepeatContainer::SetSpacing(float spacing) {
    if (auto* c = dynamic_cast<UIRepeatContainer*>(element)) {
        c->SetSpacing(spacing);
    }
}

void DelusiveUIRepeatContainer::Regenerate() {
    if (auto* c = dynamic_cast<UIRepeatContainer*>(element)) {
        c->RegenerateChildren();
    }
}