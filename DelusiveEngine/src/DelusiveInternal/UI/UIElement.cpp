#include <DelusiveInternal/UI/UIElement.h>
#include <DelusiveInternal/UI/DelusiveUI.h>
#include <DelusiveInternal/Core/DelusiveRegistry.h>
#include <DelusiveInternal/Utils/DelusiveMacros.h>
#include <DelusiveInternal/Rendering/DelusiveRenderer.h>
#include <DelusiveInternal/UI/UICanvas.h>

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
    // --- Base Properties ---
    registry->DrawImGui();

    ImGui::SeparatorText("Children");

    // Handle child removal outside the loop to avoid iterator invalidation
    int removeIndex = -1;

    // --- Draw Each Child ---
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        if (!child) continue;

        // Unique ID scope for ImGui elements
        ImGui::PushID(static_cast<int>(i));

        // A bordered area for each child (gives a visual boundary)
        ImGui::BeginChild(
            "ChildElement",
            ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 10), // height scales to content
            true,
            ImGuiWindowFlags_MenuBar
        );

        // Optional header bar inside each child block
        if (ImGui::BeginMenuBar()) {
            ImGui::TextUnformatted(child->GetType().c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.25f, 0.25f, 1.0f));
            if (ImGui::Button("Remove")) {
                removeIndex = static_cast<int>(i);
            }
            ImGui::PopStyleColor();
            ImGui::EndMenuBar();
        }

        // Add padding to separate header from content
        ImGui::Dummy(ImVec2(0, 4));

        // Draw the child’s own inspector
        child->DrawImGui();

        ImGui::EndChild();
        ImGui::PopID();

        // Add spacing between child blocks
        ImGui::Dummy(ImVec2(0, 5));
    }

    // --- Handle Child Removal ---
    if (removeIndex >= 0 && removeIndex < (int)children.size()) {
        children.erase(children.begin() + removeIndex);
    }

    // --- Add Child Button ---
    ImGui::Separator();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
    if (ImGui::Button("+ Add Child Element", ImVec2(-FLT_MIN, 0))) {
        ImGui::OpenPopup("AddUIElementPopup");
    }

    if (ImGui::BeginPopup("AddUIElementPopup")) {
        std::string type = DelusiveUI::DrawUIElementAddMenu();
        if (!type.empty()) {
            auto newElement = DelusiveUI::CreateUIElementByType(type, renderer);
            if (newElement) {
                newElement->LinkCanvas(parentCanvas);
                children.push_back(std::move(newElement));
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
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

			std::unique_ptr<UIElement> child = DelusiveUI::CreateUIElementByType(typeToken, renderer);

            if (child) {
                child->Deserialize(in);
                child->LinkCanvas(parentCanvas);
                children.push_back(std::move(child));
            }
            continue;
        }
    }
}