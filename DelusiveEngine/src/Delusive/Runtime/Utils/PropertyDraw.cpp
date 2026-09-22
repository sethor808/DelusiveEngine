#include <Delusive/Runtime/Utils/PropertyDraw.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <imgui/imgui.h>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <iostream>

//==== [ZONE: PropertyDraw — extracted from Property.inl DrawImGui] ====

void DrawTextureUI(DelusiveTexture& texture, const std::string& name)
{
    ImGui::Text("Texture: %s", std::filesystem::path(texture.texturePath).filename().string().c_str());
    if (ImGui::Button(("Change Texture##" + name).c_str())) {
        ImGui::OpenPopup(("TextureBrowser##" + name).c_str());
    }
    if (ImGui::BeginPopup(("TextureBrowser##" + name).c_str())) {
        std::function<void(const std::filesystem::path&)> DrawDirectory;
        DrawDirectory = [&](const std::filesystem::path& path) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory()) {
                    if (ImGui::BeginMenu((entry.path().filename().string() + "/").c_str())) {
                        DrawDirectory(entry.path());
                        ImGui::EndMenu();
                    }
                }
                else if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                        std::string filename = entry.path().filename().string();
                        if (ImGui::Selectable(filename.c_str())) {
                            texture.texturePath = entry.path().string();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
            }
            };
        DrawDirectory(SPRITE_FOLDER);
        ImGui::EndPopup();
    }
}

void DrawFontUI(DelusiveFont& font, const std::string& name)
{
    ImGui::Text("Font: %s", std::filesystem::path(font.fontPath).filename().string().c_str());
    ImGui::DragFloat(("Size##" + name).c_str(), &font.fontSize, 1.0f, 6.0f, 128.0f);
    if (ImGui::Button(("Change Font##" + name).c_str())) {
        ImGui::OpenPopup(("FontBrowser##" + name).c_str());
    }
    if (ImGui::BeginPopup(("FontBrowser##" + name).c_str())) {
        for (auto& entry : std::filesystem::recursive_directory_iterator(FONT_FOLDER)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".ttf" || ext == ".otf") {
                    std::string filename = entry.path().filename().string();
                    if (ImGui::Selectable(filename.c_str())) {
                        font.fontPath = entry.path().string();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        }
        ImGui::EndPopup();
    }
}

void DrawLinkUI(DelusiveLink<Agent>& link, const std::string& name)
{
    std::string displayName;
    if (!link.id.IsValid())
        displayName = "<None>";
    else if (link.cached)
        displayName = link.cached->GetName();
    else
        displayName = link.id.ToString();

    ImGui::Button((name + ": " + displayName).c_str());

    //Allows drop payload
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("DND_AGENT_UUID"))
        {
            UUID droppedUUID =
                *reinterpret_cast<const UUID*>(payload->Data);

            link.id = droppedUUID;
            link.cached = nullptr;
            link.dirty = true;
        }

        ImGui::EndDragDropTarget();
    }

    //Right-click to clear
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Clear"))
        {
            link.id = UUID{};
            link.cached = nullptr;
            link.dirty = true;
        }
        ImGui::EndPopup();
    }
}

void DrawUILinkUI(DelusiveUILink& link, const std::string& name)
{
    UIElement* resolved = nullptr;

    if (link.id.IsValid() && link.canvasLink) {
        resolved = link.canvasLink->FindElementByUUID(link.id);
    }

    std::string displayName;
    if (!link.id.IsValid())
        displayName = "<None>";
    else if (!link.canvasLink)
        displayName = "<Canvas Missing>";
    else if (!resolved)
        displayName = "<Missing UIElement>";
    else
        displayName = resolved->GetName();

    ImGui::Button((name + ": " + displayName).c_str());

    //Allows drop payload
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("UI_ELEMENT_UUID"))
        {
            UUID droppedUUID =
                *reinterpret_cast<const UUID*>(payload->Data);

            link.id = droppedUUID;
            link.dirty = true;
        }

        ImGui::EndDragDropTarget();
    }

    //Right-click to clear
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Clear"))
        {
            link.id = UUID{};
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

        ImGui::Text("UUID: %s", owned.getID().ToString().c_str());

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
