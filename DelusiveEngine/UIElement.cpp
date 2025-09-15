#include "UIElement.h"
#include "DelusiveUI.h"

UIElement::UIElement(DelusiveRenderer& _renderer)
    : renderer(_renderer)
{
	RegisterProperties();
}

void UIElement::RegisterProperties(){
	registry.Register("name", &name);
	registry.Register("enabled", &enabled);
	registry.Register("position", &position);
	registry.Register("size", &size);
}

void UIElement::DrawImGui() {
	registry.DrawImGui();
}

void UIElement::Serialize(std::ostream& out) const{
	out << "[UIElement " << this->GetType() << "]\n";
	registry.Serialize(out);

	// Serialize elements here
	for (auto& elem : children) {
		elem->Serialize(out);
	}

	out << "[/UIElement]\n";
}

void UIElement::Deserialize(std::istream& in) {
    registry.Deserialize(in); // will stop at first header

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line == "[/UIElement]") break;

        // If a child block starts (e.g. nested elements in UIPanel)
        if (line.rfind("[UIElement", 0) == 0) {
            std::istringstream iss(line);
            std::string tag, typeToken;
            iss >> tag >> typeToken;
            if (!typeToken.empty() && typeToken.back() == ']') typeToken.pop_back();

            std::unique_ptr<UIElement> child;
            if (typeToken == "UILabel")       child = std::make_unique<UILabel>(renderer);
            else if (typeToken == "UIImage")  child = std::make_unique<UIImage>(renderer);
            else if (typeToken == "UIButton") child = std::make_unique<UIButton>(renderer);
            else if (typeToken == "UIPanel")  child = std::make_unique<UIPanel>(renderer);

            if (child) {
                child->Deserialize(in);
                children.push_back(std::move(child));
            }
            continue;
        }
    }
}