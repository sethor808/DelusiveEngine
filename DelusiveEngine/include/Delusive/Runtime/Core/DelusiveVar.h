#pragma once
//HEADERS INCLUDING THIS FILE MUST INCLUDE DelusiveRegistry.h IN THEIR SRC FILE!!!

class PropertyRegistry;

template<typename T>
class DelusiveVar {
public:
    DelusiveVar(PropertyRegistry&, const char* name, T* value);
};

#define DELUSIVE_VAR(type, name, value, registryHandle) \
    type name = value; \
    DelusiveVar<type> _field_##name{registryHandle, #name, &name};