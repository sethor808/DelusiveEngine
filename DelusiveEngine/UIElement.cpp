#include "UIElement.h"
#include "DelusiveUI.h"

UIElement::UIElement() {
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
	out << "[UIElement " << GetType() << "]\n";
	registry.Serialize(out);

	// Serialize elements here
	for (auto& elem : children) {
		elem->Serialize(out);
	}

	out << "[/UIElement]\n";
}

void UIElement::Deserialize(std::istream& in) {
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;

		if (line == "[/UIElement]") {
			break; // finished this agent block
		}

		// Components
		if (line.rfind("[UIElement", 0) == 0) {
			std::istringstream iss(line);
			std::string discard, type;
			iss >> discard >> type; // [Component SpriteComponent]

			if (type.back() == ']') {
				type.pop_back();
			}

			UIElement* child = nullptr;
			if (type == "UIImage")        child = AddChild<UIImage>();
			else if(type == "UIButton")  child = AddChild<UIButton>();
			else if(type == "UIText")    child = AddChild<UILabel>();
			else if(type == "UIPanel")   child = AddChild<UIPanel>();
			else {
				std::cerr << "[UIElement::Deserialize] Unknown UIElement type: " << type << "\n";
			}
			if (child) {
				child->Deserialize(in); // consumes until [/Component]
			}
		}
		else {
			// Allow both "key=value" and "key value"
			std::string key, value;
			auto pos = line.find('=');
			if (pos != std::string::npos) {
				key = line.substr(0, pos);
				value = line.substr(pos + 1);
			}
			else {
				std::istringstream iss(line);
				iss >> key;
				std::getline(iss, value);
				if (!value.empty() && value[0] == ' ') value.erase(0, 1); // trim leading space
			}

			if (key.empty()) continue;

			// Try registry first
			std::istringstream valStream(value);
			bool handled = false;
			for (auto& p : registry.properties) {
				if (p->name == key) {
					p->Deserialize(valStream);
					handled = true;
					break;
				}
			}
			if (handled) continue;
		}
	}
}