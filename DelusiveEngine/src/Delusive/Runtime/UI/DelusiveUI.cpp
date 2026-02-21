#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/UILabel.h>
#include <Delusive/Runtime/UI/UIButton.h>
#include <Delusive/Runtime/UI/UIImage.h>
#include <Delusive/Runtime/UI/UIPanel.h>
#include <Delusive/Runtime/UI/UITalismanDisplay.h>
#include <Delusive/Runtime/UI/UITalismanButton.h>
#include <Delusive/Runtime/UI/UIEquipScreen.h>
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <imgui/imgui.h>

std::string DelusiveUI::DrawUIElementAddMenu() {
    std::string selectedType;

    if (ImGui::BeginMenu("Basic")) {
        if (ImGui::MenuItem("UILabel")) selectedType = "UILabel";
        if (ImGui::MenuItem("UIButton")) selectedType = "UIButton";
        if (ImGui::MenuItem("UIImage")) selectedType = "UIImage";
        if (ImGui::MenuItem("UIPanel")) selectedType = "UIPanel";
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Advanced")) {
        if (ImGui::MenuItem("UITalismanDisplay")) selectedType = "UITalismanDisplay";
        if (ImGui::MenuItem("UITalismanButton")) selectedType = "UITalismanButton";
        if (ImGui::MenuItem("UIEquipScreen")) selectedType = "UIEquipScreen";
        if (ImGui::MenuItem("UIRepeatContainer")) selectedType = "UIRepeatContainer";
        if (ImGui::MenuItem("UIScriptContainer")) selectedType = "UIScriptContainer";
        ImGui::EndMenu();
    }

    return selectedType;
}

std::unique_ptr<UIElement> DelusiveUI::CreateUIElementByType(const std::string& type, DelusiveRenderer& renderer) {
     if (type == "UILabel") {
         return std::make_unique<UILabel>(renderer);
     }
     else if (type == "UIButton") {
         return std::make_unique<UIButton>(renderer);
     }
     else if (type == "UIImage") {
         return std::make_unique<UIImage>(renderer);
     }
     else if (type == "UIPanel") {
         return std::make_unique<UIPanel>(renderer);
     }
     else if (type == "UITalismanDisplay") {
         return std::make_unique<UITalismanDisplay>(renderer);
     }
     else if (type == "UIEquipScreen") {
         return std::make_unique<UIEquipScreen>(renderer);
     }
     else if (type == "UIRepeatContainer") {
         return std::make_unique<UIRepeatContainer>(renderer);
     }
     else if (type == "UIScriptContainer") {
         return std::make_unique<UIScriptContainer>(renderer);
     }

     // Unknown type fallback
     return nullptr;
}