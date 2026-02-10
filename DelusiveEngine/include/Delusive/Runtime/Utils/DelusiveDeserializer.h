#pragma once
#include <sstream>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class DelusiveDeserializer {
    std::unordered_map<std::string, std::string> entries;
public:
    DelusiveDeserializer(std::istream& in) {
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            auto pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            Trim(key);
            Trim(val);
            entries[key] = val;
        }
    }

    bool Has(const std::string& key) const {
        return entries.find(key) != entries.end();
    }

    std::string ReadString(const std::string& key, const std::string& def = "") const {
        auto it = entries.find(key);
        return it != entries.end() ? it->second : def;
    }

    int ReadInt(const std::string& key, int def = 0) const {
        auto it = entries.find(key);
        return it != entries.end() ? std::stoi(it->second) : def;
    }

    float ReadFloat(const std::string& key, float def = 0.0f) const {
        auto it = entries.find(key);
        return it != entries.end() ? std::stof(it->second) : def;
    }

    bool ReadBool(const std::string& key, bool def = false) const {
        auto it = entries.find(key);
        if (it == entries.end()) return def;
        return (it->second == "1" || it->second == "true" || it->second == "True");
    }

    glm::vec2 ReadVec2(const std::string& key, glm::vec2 def = { 0,0 }) const {
        auto it = entries.find(key);
        if (it == entries.end()) return def;
        glm::vec2 v;
        std::stringstream ss(it->second);
        ss >> v.x >> v.y;
        return v;
    }

    glm::vec3 ReadVec3(const std::string& key, glm::vec3 def = { 0,0,0 }) const {
        auto it = entries.find(key);
        if (it == entries.end()) return def;
        glm::vec3 v;
        std::stringstream ss(it->second);
        ss >> v.x >> v.y >> v.z;
        return v;
    }

    glm::vec4 ReadVec4(const std::string& key, glm::vec4 def = { 0,0,0,0 }) const {
        auto it = entries.find(key);
        if (it == entries.end()) return def;
        glm::vec4 v;
        std::stringstream ss(it->second);
        ss >> v.x >> v.y >> v.z >> v.w;
        return v;
    }

private:
    static void Trim(std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) {
            s.clear();
            return;
        }
        s = s.substr(start, end - start + 1);
    }
};