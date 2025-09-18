#include <Delusive/DelusiveScriptAPI.h>
#include "BasicFollow.h"
#include <iostream>

// Macro to simplify registration
#define REGISTER_SCRIPT(REGISTRY, CLASS_NAME) \
    do { \
        REGISTRY.Register(#CLASS_NAME, [](DelusiveScriptAgent* owner) -> BehaviourScript* { \
            return new CLASS_NAME(owner); \
        }); \
    } while(0)

extern "C" DS_API void InitializeScripts(ScriptRegistry& registry) {
    REGISTER_SCRIPT(registry, BasicFollow);
    // Add more scripts here as you implement them
}

void UpdateScripts(float deltaTime) {
	std::cout << "Scripts updated, deltaTime: " << deltaTime << std::endl;
}

void ShutdownScripts() {
	std::cout << "Scripts shutdown!" << std::endl;
}