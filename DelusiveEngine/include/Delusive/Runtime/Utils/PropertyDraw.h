#pragma once
#include <string>
#include <Delusive/Runtime/Core/IDLink.h>

//==== [ZONE: PropertyDraw — editor draws for Property.inl custom types, bodies in PropertyDraw.cpp] ====
class Agent;
struct DelusiveTexture;
struct DelusiveFont;

void DrawTextureUI(DelusiveTexture&, const std::string& name);
void DrawFontUI(DelusiveFont&, const std::string& name);
void DrawLinkUI(DelusiveLink<Agent>&, const std::string& name);
void DrawUILinkUI(DelusiveUILink&, const std::string& name);
//==== [/ZONE: PropertyDraw] ====
