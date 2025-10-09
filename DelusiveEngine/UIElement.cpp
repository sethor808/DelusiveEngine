#include "UIElement.h"
#include "DelusiveUI.h"
#include "DelusiveRegistry.h"
#include "DelusiveMacros.h"
#include "DelusiveRenderer.h"

UIElement::UIElement(DelusiveRenderer& _renderer)
    : renderer(_renderer), registry(std::make_unique<PropertyRegistry>())
{
	RegisterProperties();
}

UIElement::~UIElement() = default;

void UIElement::RegisterProperties(){
	registry->Register("name", &name);
	registry->Register("enabled", &enabled);
	registry->Register("position", &position);
	registry->Register("size", &size);
}

std::vector<UIElement*> UIElement::GetChildren() {
    std::vector<UIElement*> result;
    result.reserve(children.size());

    for (auto& c : children) {
        if (!c) continue;
        result.push_back(c.get());
    }
    return result;
}

void UIElement::DrawImGui() {
	registry->DrawImGui();
}

void UIElement::Serialize(std::ostream& out) const{
	out << "[UIElement " << this->GetType() << "]\n";
	registry->Serialize(out);

	// Serialize elements here
	for (auto& elem : children) {
		elem->Serialize(out);
	}

	out << "[/UIElement]\n";
}

void UIElement::Deserialize(std::istream& in) {
    registry->Deserialize(in); // will stop at first header

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line == "[/UIElement]") return;

        // If a child block starts (e.g. nested elements in UIPanel)
        if (line.rfind("[UIElement", 0) == 0) {
            std::istringstream iss(line);
            std::string discard, typeToken;
            iss >> discard >> typeToken;
            if (!typeToken.empty() && typeToken.back() == ']') typeToken.pop_back();

            std::unique_ptr<UIElement> child;
            if (typeToken == "UILabel")       child = std::make_unique<UILabel>(renderer);
            else if (typeToken == "UIImage")  child = std::make_unique<UIImage>(renderer);
            else if (typeToken == "UIButton") child = std::make_unique<UIButton>(renderer);
            else if (typeToken == "UIPanel")  child = std::make_unique<UIPanel>(renderer);
            else if (typeToken == "UITalismanDisplay") child = std::make_unique<UITalismanDisplay>(renderer);

            if (child) {
                child->Deserialize(in);
                child->LinkCanvas(parentCanvas);
                children.push_back(std::move(child));
            }
            continue;
        }
    }
}