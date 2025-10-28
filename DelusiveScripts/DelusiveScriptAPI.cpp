#include <DelusiveExternal/DelusiveScriptAPI.h>
#include "DelusiveScripts.h"
#include <iostream>

// Macro to simplify registration
#define REGISTER_ENEMYLOGIC(REGISTRY, CLASS_NAME) \
    do { \
        REGISTRY.Register(#CLASS_NAME, [](DelusiveScriptAgent* owner) -> BehaviourScript* { \
            return new CLASS_NAME(owner); \
        }); \
    } while(0)

#define REGISTER_UISCRIPT(REGISTRY, CLASS_NAME) \
    do { \
        REGISTRY.Register(#CLASS_NAME, [](UIScript* owner) -> UIScript* { \
            return new CLASS_NAME(owner); \
        }); \
    } while(0)

extern "C" DS_API void InitializeScripts(ScriptRegistry& registry) {

}

void UpdateScripts(float deltaTime) {
	std::cout << "Scripts updated, deltaTime: " << deltaTime << std::endl;
}

void ShutdownScripts() {
	std::cout << "Scripts shutdown!" << std::endl;
}