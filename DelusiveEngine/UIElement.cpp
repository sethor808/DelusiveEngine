#include "UIElement.h"
#include "DelusiveUI.h"
#include "DelusiveRegistry.h"
#include "DelusiveMacros.h"
#include "DelusiveRenderer.h"
#include "UICanvas.h"

UIElement::UIElement(DelusiveRenderer& _renderer)
    : renderer(_renderer), registry(std::make_unique<PropertyRegistry>()), id(UUID::GenerateRandom())
{
	RegisterProperties();
}

UIElement::~UIElement() {
    if(parentCanvas) {
        parentCanvas->idManager.Unregister(id);
	}
}


void UIElement::LinkCanvas(UICanvas* canvas) {
    parentCanvas = canvas;
    for (auto& child : children) {
        if (child) {
            child->LinkCanvas(canvas);
        }
	}

    //Just for safe measures - probably extra code
	if (parentCanvas == nullptr) return;

	//Generate ID here since we may not be linked upwards until after construction
	parentCanvas->idManager.Register(this, id);
}

void UIElement::RegisterProperties(){
	registry->Register("id", &id);
	registry->Register("name", &name);
	registry->Register("enabled", &enabled);
	registry->Register("position", &position);
	registry->Register("size", &size);
}

std::vector<UIElement*> UIElement::GetChildren() {
    std::vector<UIElement*> refs;
    refs.reserve(children.size());
    
    for (auto& c : children) {
        refs.push_back(c.get());
    }

    return refs;
}

void UIElement::DrawImGui() {
	registry->DrawImGui();

    for(auto& child : children) {
        ImGui::Separator();
        if (child) {
            child->DrawImGui();
        }
	}
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

    if (parentCanvas) {
        parentCanvas->idManager.Register(this, id);
	}

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
            else if (typeToken == "UITalismanButton") child = std::make_unique<UITalismanButton>(renderer);
            else if (typeToken == "UIEquipScreen") child = std::make_unique<UIEquipScreen>(renderer);
            else if (typeToken == "UIRepeatContainer") child = std::make_unique<UIRepeatContainer>(renderer);

            if (child) {
                child->Deserialize(in);
                child->LinkCanvas(parentCanvas);
                children.push_back(std::move(child));
            }
            continue;
        }
    }
}