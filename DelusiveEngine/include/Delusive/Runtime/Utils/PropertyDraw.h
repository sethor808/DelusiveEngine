#pragma once
#include <string>
#include <initializer_list>
#include <Delusive/Runtime/Core/IDLink.h>

//==== [ZONE: PropertyDraw — editor draws for Property.inl custom types, bodies in PropertyDraw.cpp] ====
struct DelusiveInstance;
class Agent;
class Scene;
class Component;
class UIElement;
class BehaviourScript;
class UIScript;
struct DelusiveTexture;
struct DelusiveFont;

//The pickers need an instance to reach the factories, but DrawImGui() takes no
//arguments and threading it through would touch every registry->DrawImGui() call.
//The editor is single threaded, so it sets this once at startup instead.
namespace DelusiveEditorContext {
    void SetInstance(DelusiveInstance*);
    DelusiveInstance* GetInstance();
	//The scene being edited, for pickers that list its agents
	void SetScene(Scene*);
	Scene* GetScene();
}

//Popup listing the files under folder with a filter - true on the frame one is picked
bool DrawFilePicker(const char* popupID, const char* folder, std::initializer_list<const char*> extensions, std::string& path);
bool DrawImagePicker(const char* popupID, std::string& path);
void DrawTextureUI(DelusiveTexture&, const std::string& name);
void DrawFontUI(DelusiveFont&, const std::string& name);
void DrawLinkUI(DelusiveLink<Agent>&, const std::string& name);
void DrawUILinkUI(DelusiveUILink&, const std::string& name);
//Hidden behind a click - ids are only wanted when debugging
void DrawIDUI(const UUID&, const std::string& name);

//Owned object pickers - one per factory-backed type
void DrawObjectUI(DelusiveObject<Component>&, const std::string& name);
void DrawObjectUI(DelusiveObject<UIElement>&, const std::string& name);
void DrawObjectUI(DelusiveObject<BehaviourScript>&, const std::string& name);
void DrawObjectUI(DelusiveObject<UIScript>&, const std::string& name);
//==== [/ZONE: PropertyDraw] ====
