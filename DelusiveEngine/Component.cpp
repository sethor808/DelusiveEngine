#include "Component.h"

void Component::SetName(const char* newName) {
	if (newName[0] == '\0') {
		strncpy_s(name, "New Component", sizeof(name));
	}

	strncpy_s(name, newName, sizeof(name));
}