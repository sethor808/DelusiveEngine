#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <DelusiveExternal/UUID.h>

class UIElement;
class DelusiveUICanvas;

class ScriptUIElement {
public:
    virtual void Link(UIElement* element) { this->element = element; }
    virtual UIElement* GetLink() { return this->element; }

    virtual bool IsValid() const { return element != nullptr; }
    virtual bool SupportsClick();
    virtual void SetOnClick(std::function<void()>) {}

    DelusiveUICanvas GetCanvas();

    virtual const UUID& GetID() const;
    virtual std::string GetType() const;
    virtual std::string GetName() const;

    virtual bool IsEnabled() const;
    virtual void SetEnabled(bool);

    virtual glm::vec2 GetSize() const;
    virtual glm::vec2 GetPosition() const;

    virtual void SetSize(const glm::vec2&);
    virtual void SetPosition(const glm::vec2&);

    virtual std::vector<ScriptUIElement> GetChildren() const;

protected:
    UIElement* element = nullptr; //non-owning
};