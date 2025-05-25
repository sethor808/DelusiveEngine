#include "Component.h"

char* Component::GetName() {
	return name;
}

void Component::SetName(char* newName) {
	if (newName[0] == '\0') {
		strncpy_s(name, "New Component", sizeof(name));
	}

	strncpy_s(name, newName, sizeof(name));
}