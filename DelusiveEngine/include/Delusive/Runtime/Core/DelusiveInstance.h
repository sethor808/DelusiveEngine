#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>

class DelusiveRenderer;
class DelusiveAudio;
class UUIDManager;
class ScriptManager;
class DelusiveLibrary;

struct DelusiveInstance {
    DelusiveRenderer& renderer;
    DelusiveAudio& audio;
    UUIDManager& uuidManager;
    ScriptManager& scriptManager;
    DelusiveLibrary& delusiveLibrary;
};