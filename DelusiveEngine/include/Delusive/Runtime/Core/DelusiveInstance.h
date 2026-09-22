#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>

class DelusiveRenderer;
class UUIDManager;
class ScriptManager;
class DelusiveLibrary;

struct DelusiveInstance {
    DelusiveRenderer& renderer;
    UUIDManager& uuidManager;
    ScriptManager& scriptManager;
    DelusiveLibrary& delusiveLibrary;
};