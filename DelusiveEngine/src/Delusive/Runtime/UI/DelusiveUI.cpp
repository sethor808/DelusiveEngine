#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/UILabel.h>
#include <Delusive/Runtime/UI/UIButton.h>
#include <Delusive/Runtime/UI/UIImage.h>
#include <Delusive/Runtime/UI/UIPanel.h>
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Talismans/Talisman.h>
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
        //if (ImGui::MenuItem("UIEquipScreen")) selectedType = "UIEquipScreen";
        if (ImGui::MenuItem("UIRepeatContainer")) selectedType = "UIRepeatContainer";
        if (ImGui::MenuItem("UIScriptContainer")) selectedType = "UIScriptContainer";
        ImGui::EndMenu();
    }

    return selectedType;
}

std::unique_ptr<UIElement> DelusiveUI::CreateUIElementByType(const std::string& type, DelusiveInstance& instance) {
     if (type == "UILabel") {
         return std::make_unique<UILabel>(instance);
     }
     else if (type == "UIButton") {
         return std::make_unique<UIButton>(instance);
     }
     else if (type == "UIImage") {
         return std::make_unique<UIImage>(instance);
     }
     else if (type == "UIPanel") {
         return std::make_unique<UIPanel>(instance);
     }
     else if (type == "UIRepeatContainer") {
         return std::make_unique<UIRepeatContainer>(instance);
     }
     else if (type == "UIScriptContainer") {
         return std::make_unique<UIScriptContainer>(instance);
     }

     // Unknown type fallback
     return nullptr;
}

const std::vector<std::string>& DelusiveUI::ListBindSources() {
	static const std::vector<std::string> sources = { "PlayerTalismans" };
	return sources;
}

const std::vector<UIBindField>& DelusiveUI::ListBindFields() {
	static const std::vector<UIBindField> fields = {
		{ "base", "texture of the talisman" },
		{ "glyph", "texture of the talisman's glyph" },
		{ "string", "texture of one string" },
		{ "hp", "strings left on the talisman (a number)" },
		{ "maxHP", "strings the talisman starts with (a number)" },
		{ "active", "1 for the talisman the next hit damages, otherwise 0" }
	};
	return fields;
}

//Fetched every update, so a source should stay cheap. Items that compare equal skip the rebuild.
std::vector<UIBindItem> DelusiveUI::FetchBindSource(const std::string& source, UICanvas& canvas) {
	std::vector<UIBindItem> items;

	PlayerAgent* player = canvas.FetchPlayer();
	if (!player) return items;

	if (source == "PlayerTalismans") {
		//The stack top: the next hit damages this one
		Talisman* active = player->GetActiveTalisman();

		for (const auto& talisman : player->GetTalismans()) {
			//A broken talisman is gone from the display
			if (talisman->GetIsBroken()) continue;

			items.push_back({
				{ "base", talisman->GetBaseTexture() },
				{ "glyph", talisman->GetGlyphTexture() },
				{ "string", talisman->GetStringTexture() },
				{ "hp", std::to_string(talisman->GetCurrentHP()) },
				{ "maxHP", std::to_string(talisman->GetMaxHP()) },
				{ "active", talisman.get() == active ? "1" : "0" }
			});
		}
	}

	return items;
}

const std::string* DelusiveUI::FetchBindField(const UIBindItem& item, const std::string& field) {
	if (field.empty()) return nullptr;

	auto found = item.find(field);
	return found != item.end() ? &found->second : nullptr;
}
