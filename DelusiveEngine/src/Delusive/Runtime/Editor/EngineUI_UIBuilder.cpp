#include <Delusive/Runtime/Editor/EngineUI.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <imgui/imgui.h>

void EngineUI::RenderUIBuilder(Scene& scene) {
    if (!uiRegistry) {
        uiRegistry = std::make_unique<DelusiveUIRegistry>(instance);
        //TODO: restore canvas load once DelusiveUIRegistry has its factory-based file I/O
    }

    ImGui::Begin("UIBuilderPanel");

    // Canvas toolbar: pick, create, save
    const char* activeLabel = editingCanvas ? editingCanvas->GetName().c_str() : "<None>";
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::BeginCombo("Canvas", activeLabel)) {
        for (const auto& name : uiRegistry->GetAllNames()) {
            bool isSelected = editingCanvas && editingCanvas->GetName() == name;
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                editingCanvas = uiRegistry->Get(name);
                selectedUIElement = nullptr;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputText("##NewCanvasName", uiCanvasNameBuffer, sizeof(uiCanvasNameBuffer));
    ImGui::SameLine();
    if (ImGui::Button("New Canvas")) {
        std::string name = uiCanvasNameBuffer;
        if (!name.empty() && !uiRegistry->Exists(name)) {
            auto canvas = std::make_unique<UICanvas>(instance);
            canvas->SetName(name);
            uiRegistry->Register(std::move(canvas));
            editingCanvas = uiRegistry->Get(name);
            selectedUIElement = nullptr;
            uiCanvasNameBuffer[0] = '\0';
        }
    }

    ImGui::SameLine();
    ImGui::BeginDisabled();
    ImGui::Button("Save All"); //TODO: re-enable with DelusiveUIRegistry file I/O
    ImGui::EndDisabled();

    ImGui::Separator();

    if (!editingCanvas) {
        ImGui::TextDisabled("Select or create a canvas to begin.");
        ImGui::End();
        return;
    }

    // Hierarchy
    if (ImGui::BeginChild("UIHierarchy", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true)) {
        ImGui::Text("%s", editingCanvas->GetName().c_str());

        if (ImGui::BeginPopupContextWindow("CanvasRightClick", ImGuiPopupFlags_MouseButtonRight)) {
            std::string type = DelusiveUI::DrawUIElementAddMenu();
            if (!type.empty()) {
                auto element = DelusiveUI::CreateUIElementByType(type, instance);
                if (element) {
                    element->LinkCanvas(editingCanvas);
                    editingCanvas->AddElement(std::move(element));
                }
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();
        for (UIElement* element : editingCanvas->GetElements()) {
            DrawUIElementNode(element);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Viewport (Stage 2: render canvas to framebuffer + spatial drag/resize)
    if (ImGui::BeginChild("UIViewport", ImVec2(ImGui::GetContentRegionAvail().x * 0.55f, 0), true)) {
        ImGui::TextDisabled("Viewport");
        ImGui::TextDisabled("Spatial editing lands in Stage 2.");
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Inspector (reuses the registered-property DrawImGui path)
    if (ImGui::BeginChild("UIInspector", ImVec2(0, 0), true)) {
        if (selectedUIElement) {
            selectedUIElement->DrawImGui();
        }
        else {
            ImGui::TextDisabled("Nothing selected.");
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void EngineUI::DrawUIElementNode(UIElement* element) {
    if (!element) return;

    std::vector<UIElement*> children = element->GetChildren();

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        (children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) |
        (selectedUIElement == element ? ImGuiTreeNodeFlags_Selected : 0);

    std::string label = element->GetName().empty() ? element->GetType() : element->GetName();

    ImGui::PushID(element);
    bool open = ImGui::TreeNodeEx(label.c_str(), flags);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selectedUIElement = element;
    }

    if (ImGui::BeginPopupContextItem("ElementRightClick", ImGuiPopupFlags_MouseButtonRight)) {
        std::string type = DelusiveUI::DrawUIElementAddMenu();
        if (!type.empty()) {
            auto child = DelusiveUI::CreateUIElementByType(type, instance);
            if (child) {
                child->LinkCanvas(editingCanvas);
                element->AddChild(std::move(child));
            }
        }
        ImGui::EndPopup();
    }

    if (open) {
        for (UIElement* child : children) {
            DrawUIElementNode(child);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}
