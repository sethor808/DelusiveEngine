#include <Delusive/Runtime/Utils/PropertyDraw.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <imgui/imgui.h>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <iostream>
#include <vector>

//==== [ZONE: PropertyDraw — extracted from Property.inl DrawImGui] ====

namespace {
	//Only one picker is open at a time, so they share the list and filter
	std::vector<std::string> pickerFiles;
	char pickerFilter[64] = "";

	std::string Lowered(std::string text) {
		std::transform(text.begin(), text.end(), text.begin(), ::tolower);
		return text;
	}

	void SetAgentLink(DelusiveLink<Agent>& link, const UUID& id) {
		link.id = id;
		link.cached = nullptr;
		link.dirty = true;
	}

	std::string ElementLabel(UIElement& element) {
		return element.GetName().empty() ? std::string(element.GetType()) : element.GetName();
	}

	//Saved children only - the ones the UI Builder hierarchy shows
	std::vector<UIElement*> SavedChildren(UIElement& element) {
		std::vector<UIElement*> children = element.GetChildren();
		children.resize(element.SavedChildCount());
		return children;
	}
}

bool DrawFilePicker(const char* popupID, const char* folder, std::initializer_list<const char*> extensions, std::string& path) {
	if (!ImGui::BeginPopup(popupID)) return false;

	if (ImGui::IsWindowAppearing()) {
		pickerFiles.clear();
		pickerFilter[0] = '\0';
		std::error_code ec;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(folder, ec)) {
			if (!entry.is_regular_file()) continue;
			const std::string extension = Lowered(entry.path().extension().string());
			for (const char* wanted : extensions) {
				if (extension == wanted) pickerFiles.push_back(entry.path().generic_string());
			}
		}
		std::sort(pickerFiles.begin(), pickerFiles.end());
		ImGui::SetKeyboardFocusHere();
	}

	ImGui::InputText("Filter", pickerFilter, sizeof(pickerFilter));
	const std::string filter = Lowered(pickerFilter);

	bool picked = false;
	ImGui::BeginChild("files", ImVec2(420.0f, 300.0f), ImGuiChildFlags_Borders);
	const std::string root = std::filesystem::path(folder).generic_string();
	for (const std::string& file : pickerFiles) {
		const std::string shown = file.rfind(root, 0) == 0 ? file.substr(root.size()) : file;
		if (!filter.empty() && Lowered(shown).find(filter) == std::string::npos) continue;

		if (ImGui::Selectable(shown.c_str(), file == path)) {
			path = file;
			picked = true;
			ImGui::CloseCurrentPopup();
		}
	}
	if (pickerFiles.empty()) ImGui::TextDisabled("No files in %s", folder);
	ImGui::EndChild();

	ImGui::EndPopup();
	return picked;
}

bool DrawImagePicker(const char* popupID, std::string& path) {
	return DrawFilePicker(popupID, SPRITE_FOLDER, { ".png", ".jpg", ".jpeg", ".bmp", ".tga" }, path);
}

void DrawTextureUI(DelusiveTexture& texture, const std::string& name) {
	ImGui::Text("Texture: %s", std::filesystem::path(texture.texturePath).filename().string().c_str());
	const std::string popupID = "TextureBrowser##" + name;
	if (ImGui::Button(("Change Texture##" + name).c_str())) ImGui::OpenPopup(popupID.c_str());
	DrawImagePicker(popupID.c_str(), texture.texturePath);
}

void DrawFontUI(DelusiveFont& font, const std::string& name) {
	ImGui::Text("Font: %s", std::filesystem::path(font.fontPath).filename().string().c_str());
	ImGui::DragFloat(("Size##" + name).c_str(), &font.fontSize, 1.0f, 6.0f, 128.0f);
	const std::string popupID = "FontBrowser##" + name;
	if (ImGui::Button(("Change Font##" + name).c_str())) ImGui::OpenPopup(popupID.c_str());
	DrawFilePicker(popupID.c_str(), FONT_FOLDER, { ".ttf", ".otf" }, font.fontPath);
}

void DrawIDUI(const UUID& id, const std::string& name) {
	//Collapsed by default - only wanted when debugging
	if (!ImGui::TreeNode((name + "##idNode").c_str())) return;
	ImGui::TextDisabled("%s", id.ToString().c_str());
	ImGui::SameLine();
	if (ImGui::SmallButton("Copy")) ImGui::SetClipboardText(id.ToString().c_str());
	ImGui::TreePop();
}

void DrawLinkUI(DelusiveLink<Agent>& link, const std::string& name) {
	Scene* scene = DelusiveEditorContext::GetScene();
	Agent* linked = link.cached;
	if (!linked && link.id.IsValid() && scene) linked = scene->FindAgentByUUID(link.id);
	const std::string shown = !link.id.IsValid() ? "<None>" : linked ? linked->GetName() : "(missing)";

	const std::string popupID = "LinkPicker##" + name;
	if (ImGui::Button((name + ": " + shown + "##" + name).c_str())) ImGui::OpenPopup(popupID.c_str());
	ImGui::SetItemTooltip("Click to pick, or drag an agent here from the hierarchy. Right-click to clear");

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_AGENT_UUID")) {
			SetAgentLink(link, *static_cast<const UUID*>(payload->Data));
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Clear")) SetAgentLink(link, UUID{});
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup(popupID.c_str())) {
		if (ImGui::Selectable("<None>", !link.id.IsValid())) SetAgentLink(link, UUID{});
		if (!scene) ImGui::TextDisabled("No scene open");
		else {
			for (auto& agent : scene->GetAgents()) {
				if (!agent) continue;
				ImGui::PushID(agent.get());
				const std::string label = (agent->GetName().empty() ? std::string("(unnamed)") : agent->GetName())
					+ "  [" + agent->GetType() + "]";
				if (ImGui::Selectable(label.c_str(), agent->GetID() == link.id)) SetAgentLink(link, agent->GetID());
				ImGui::PopID();
			}
		}
		ImGui::EndPopup();
	}
}

void DrawUILinkUI(DelusiveUILink& link, const std::string& name) {
	UIElement* resolved = nullptr;
	if (link.id.IsValid() && link.canvasLink) {
		resolved = link.canvasLink->FindElementByUUID(link.id);
	}

	std::string shown;
	if (!link.id.IsValid()) shown = "<None>";
	else if (!link.canvasLink) shown = "(no canvas)";
	else if (!resolved) shown = "(missing)";
	else shown = ElementLabel(*resolved);

	const std::string popupID = "UILinkPicker##" + name;
	if (ImGui::Button((name + ": " + shown + "##" + name).c_str())) ImGui::OpenPopup(popupID.c_str());
	ImGui::SetItemTooltip("Click to pick, or drag an element here. Right-click to clear");

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UI_ELEMENT_UUID")) {
			link.id = *static_cast<const UUID*>(payload->Data);
			link.dirty = true;
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Clear")) {
			link.id = UUID{};
			link.dirty = true;
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup(popupID.c_str())) {
		if (ImGui::Selectable("<None>", !link.id.IsValid())) {
			link.id = UUID{};
			link.dirty = true;
		}
		if (!link.canvasLink) ImGui::TextDisabled("Not on a canvas");

		//Nested elements are indented under their parents
		std::function<void(UIElement*)> List = [&](UIElement* element) {
			ImGui::PushID(element);
			if (ImGui::Selectable(ElementLabel(*element).c_str(), element->GetID() == link.id)) {
				link.id = element->GetID();
				link.dirty = true;
			}
			ImGui::Indent();
			for (UIElement* child : SavedChildren(*element)) List(child);
			ImGui::Unindent();
			ImGui::PopID();
		};
		if (link.canvasLink) {
			for (UIElement* element : link.canvasLink->GetElements()) List(element);
		}
		ImGui::EndPopup();
	}
}

//==== [/ZONE: PropertyDraw] ====

//==== [ZONE: owned object pickers] ====
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Scripting/UIScript.h>

namespace {
    DelusiveInstance* editorInstance = nullptr;
    Scene* editorScene = nullptr;

    //Combo over a flat name list - used where the factory has no ImGui menu of its own
    std::string DrawTypeCombo(const std::string& name, const std::vector<std::string>& types) {
        std::string picked;

        if (!ImGui::BeginCombo(("##type" + name).c_str(), "<Select type>")) return picked;

        for (const std::string& type : types) {
            if (ImGui::Selectable(type.c_str())) picked = type;
        }

        ImGui::EndCombo();
        return picked;
    }

    //Shared shell: header, inline editor for the live object, remove button.
    //DrawPicker supplies the type string when the slot is empty.
    template<typename T, typename PickerFn>
    void DrawOwnedObject(DelusiveObject<T>& owned, const std::string& name, PickerFn DrawPicker) {
        ImGui::PushID(name.c_str());
        ImGui::SeparatorText(name.c_str());

        DelusiveInstance* instance = DelusiveEditorContext::GetInstance();

        if (!owned) {
            if (!instance) {
                ImGui::TextDisabled("No editor context - cannot create.");
                ImGui::PopID();
                return;
            }

            std::string type = DrawPicker(*instance);

            if (!type.empty()) {
                std::unique_ptr<T> built = DelusiveFactory<T>::Create(type, *instance);

                if (built) {
                    //A fresh object needs an identity before it can be referenced
                    if (!built->GetID().IsValid()) built->SetID(UUID::GenerateRandom());
                    owned.set(std::move(built));
                }
            }

            ImGui::PopID();
            return;
        }

        DrawIDUI(owned.getID(), "ID");

        if (ImGui::Button("Remove")) {
            owned.object.reset();
            owned.id = UUID{};
            ImGui::PopID();
            return;
        }

        ImGui::Separator();
        owned->DrawImGui();

        ImGui::PopID();
    }
}

void DelusiveEditorContext::SetInstance(DelusiveInstance* instance) {
    editorInstance = instance;
}

DelusiveInstance* DelusiveEditorContext::GetInstance() {
    return editorInstance;
}

void DelusiveEditorContext::SetScene(Scene* scene) {
	editorScene = scene;
}

Scene* DelusiveEditorContext::GetScene() {
	return editorScene;
}

void DrawObjectUI(DelusiveObject<Component>& owned, const std::string& name) {
    DrawOwnedObject(owned, name, [](DelusiveInstance&) {
        std::string picked;
        if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddOwnedComponent");
        if (ImGui::BeginPopup("AddOwnedComponent")) {
            picked = DelusiveComponentFactory::DrawComponentAddMenu();
            ImGui::EndPopup();
        }
        return picked;
        });
}

void DrawObjectUI(DelusiveObject<UIElement>& owned, const std::string& name) {
    DrawOwnedObject(owned, name, [](DelusiveInstance&) {
        std::string picked;
        if (ImGui::Button("Add Element")) ImGui::OpenPopup("AddOwnedElement");
        if (ImGui::BeginPopup("AddOwnedElement")) {
            picked = DelusiveUI::DrawUIElementAddMenu();
            ImGui::EndPopup();
        }
        return picked;
        });
}

void DrawObjectUI(DelusiveObject<BehaviourScript>& owned, const std::string& name) {
    DrawOwnedObject(owned, name, [&name](DelusiveInstance& instance) {
        std::vector<std::string> types;
        instance.scriptManager.GetAvailableEnemyLogicScripts(types);
        return DrawTypeCombo(name, types);
        });
}

void DrawObjectUI(DelusiveObject<UIScript>& owned, const std::string& name) {
    DrawOwnedObject(owned, name, [&name](DelusiveInstance& instance) {
        std::vector<std::string> types;
        instance.scriptManager.GetAvailableUIScripts(types);
        return DrawTypeCombo(name, types);
        });
}
//==== [/ZONE: owned object pickers] ====
