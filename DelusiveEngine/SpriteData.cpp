#include "SpriteData.h"
#include <fstream>
#include <sstream>

bool SpriteData::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key;
        ss >> key;

        if (key == "texture") ss >> texturePath;
        else if (key == "scale") ss >> scale.x >> scale.y;
        else if (key == "collider_offset") ss >> colliderOffset.x >> colliderOffset.y;
        else if (key == "collider_size") ss >> colliderSize.x >> colliderSize.y;
    }

    return true;
}

bool SpriteData::SaveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "texture " << texturePath << "\n";
    file << "scale " << scale.x << " " << scale.y << "\n";
    file << "collider_offset " << colliderOffset.x << " " << colliderOffset.y << "\n";
    file << "collider_size " << colliderSize.x << " " << colliderSize.y << "\n";

    return true;
}