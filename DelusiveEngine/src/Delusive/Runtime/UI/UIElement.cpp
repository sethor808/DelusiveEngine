#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/UI/UICanvas.h>

UIElement::UIElement(DelusiveInstance& instance)
    : instance(instance), registry(std::make_unique<PropertyRegistry>()), id(UUID::GenerateRandom())
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
    registry->category = "UIElement";
    registry->type = GetType();

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
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        UUID payload = id;

        ImGui::SetDragDropPayload(
            "UI_ELEMENT_UUID",
            &payload,
            sizeof(UUID)
        );

        ImGui::Text("UI Element");
        ImGui::Text("Type: %s", GetType().c_str());
        ImGui::Text("ID: %s", id.ToString().c_str());

        ImGui::EndDragDropSource();
    }

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
            auto newElement = DelusiveUI::CreateUIElementByType(type, instance);
            if (newElement) {
                newElement->LinkCanvas(parentCanvas);
                children.push_back(std::move(newElement));
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}