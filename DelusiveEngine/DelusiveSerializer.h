#pragma once
#include <sstream>
#include <string>
#include <stack>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>

class DelusiveSerializer {
public:
    DelusiveSerializer(std::ostream& o) : out(&o) {}

    void BeginSection(const std::string& name) {
        WriteIndent();
        *out << "[" << name << "]\n";
        indent++;
    }

    void EndSection() {
        indent = std::max(0, indent - 1);
    }

    void WriteString(const std::string& key, const std::string& value) {
        WriteIndent();
        *out << key << "=" << value << "\n";
    }

    void WriteInt(const std::string& key, int value) {
        WriteIndent();
        *out << key << "=" << value << "\n";
    }

    void WriteFloat(const std::string& key, float value) {
        WriteIndent();
        *out << key << "=" << value << "\n";
    }

    void WriteBool(const std::string& key, bool value) {
        WriteIndent();
        *out << key << "=" << (value ? "1" : "0") << "\n";
    }

    void WriteVec2(const std::string& key, const glm::vec2& v) {
        WriteIndent();
        *out << key << "=" << v.x << " " << v.y << "\n";
    }

    void WriteVec3(const std::string& key, const glm::vec3& v) {
        WriteIndent();
        *out << key << "=" << v.x << " " << v.y << " " << v.z << "\n";
    }

    void WriteVec4(const std::string& key, const glm::vec4& v) {
        WriteIndent();
        *out << key << "=" << v.x << " " << v.y << " " << v.z << " " << v.w << "\n";
    }

    template<typename T>
    void WriteCustom(const std::string& key, const T& custom) {
        WriteIndent();
        *out << key << "=";
        custom.Serialize(*this);
        *out << "\n";
    }

private:
    std::ostream* out = nullptr;
    int indent = 0;

    void WriteIndent() {
        for (int i = 0; i < indent; i++)
            *out << "\t";
    }
};