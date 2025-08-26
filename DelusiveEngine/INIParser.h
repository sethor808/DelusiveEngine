#pragma once
#include "Property.h"

struct INIParser {
	static std::string Serialize(void* object, const std::string& typeName) {
		std::ostringstream out;
		out << "[" << typeName << "]\n";
		
		for (auto& prop : PropertyRegistry::Instance().Get(typeName)) {
			out << prop.name << "=" << prop.getter(object) << "\n";
		}

		return out.str();
	}

	static void Deserialize(void* object, const std::string& typeName, const std::string& data) {
		std::istringstream in(data);
		std::string line;
		bool inSection = false;

		while (std::getline(in, line)) {
			if (line.empty()) continue;

			if (line.front() == '[' && line.back() == ']') {
				inSection = (line.substr(1, line.size() - 2) == typeName);
				continue;
			}

			if (!inSection) continue;

			auto pos = line.find('=');
			if (pos == std::string::npos) continue;

			std::string key = line.substr(0, pos);
			std::string val = line.substr(pos + 1);

			for (auto& prop : PropertyRegistry::Instance().Get(typeName)) {
				if (prop.name == key) {
					prop.setter(object, val);
					break;
				}
			}
		}
	}
};