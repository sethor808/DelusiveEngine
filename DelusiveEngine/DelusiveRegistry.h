#pragma once
#include "DelusiveData.h"
#include "BehaviourRegistry.h"
#include <string>
#include <functional>
#include <filesystem>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class PropertyBase {
public:
    std::string name;
    virtual ~PropertyBase() = default;

    const std::string& GetName() const { return name; }

    virtual void Serialize(std::ostream& out) const = 0;
    virtual void Deserialize(std::istream& in) = 0;
    virtual void DrawImGui() = 0;
};

template<typename T>
class Property : public PropertyBase {
    static constexpr bool is_scalar =
        std::is_same_v<T, float> ||
        std::is_same_v<T, int> ||
        std::is_same_v<T, bool> ||
        std::is_same_v<T, glm::vec2> ||
        std::is_same_v<T, glm::vec3> ||
        std::is_same_v<T, glm::vec4> ||
        std::is_same_v<T, std::string>;

    static constexpr bool is_vector =
        std::is_same_v<T, std::vector<float>> ||
        std::is_same_v<T, std::vector<int>> ||
        std::is_same_v<T, std::vector<bool>> ||
        std::is_same_v<T, std::vector<std::string>>;

	static constexpr bool is_custom = 
        std::is_same_v<T, DelusiveTexture> ||
        std::is_same_v<T, DelusiveFont> ||
        std::is_same_v<T, DelusiveScript>;

    static_assert(
		is_scalar || is_vector || is_custom,
        "Property<T>: Unsupported type"
        );

    T* value;
public:
    Property(const std::string& n, T* val) : value(val) {
        name = n;
    }

    void Serialize(std::ostream& out) const override {
        if constexpr (is_scalar) {
            if constexpr (std::is_same<T, glm::vec2>::value) {
                out << value->x << " " << value->y;
            }
            else if constexpr (std::is_same<T, glm::vec3>::value) {
                out << value->x << " " << value->y << " " << value->z;
            }
            else if constexpr (std::is_same<T, glm::vec4>::value) {
                out << value->x << " " << value->y << " " << value->z << " " << value->w;
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                out << *value;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                out << (*value ? 1 : 0);
            }
            else {
                out << *value;
            }
        }
        else if constexpr (is_vector) {
            out << value->size();
            for (size_t i = 0; i < value->size(); i++) {
                out << " ";
                if constexpr (std::is_same_v<typename T::value_type, bool>) {
                    out << ((*value)[i] ? 1 : 0);
                }
                else {
                    out << (*value)[i];
                }
            }
        }
        else if constexpr (is_custom) {
            if constexpr (std::is_same_v<T, DelusiveTexture>) {
				out << value->texturePath;
            }
            else if constexpr (std::is_same_v<T, DelusiveFont>) {
				out << value->fontSize << " " << std::quoted(value->fontPath);
            }
            else if constexpr (std::is_same_v<T, DelusiveScript>) {
				out << value->scriptName;
            }
		}
    }

    void Deserialize(std::istream& in) override {
        if constexpr (is_scalar) {
            if constexpr (std::is_same<T, glm::vec2>::value) {
                in >> value->x >> value->y;
            }
            else if constexpr (std::is_same<T, glm::vec3>::value) {
                in >> value->x >> value->y >> value->z;
            }
            else if constexpr (std::is_same<T, glm::vec4>::value) {
                in >> value->x >> value->y >> value->z >> value->w;
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                std::string temp;
                std::getline(in, temp);
                if (!temp.empty() && temp[0] == ' ') temp.erase(0, 1);
                *value = temp;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                int tmp;
                in >> tmp;
                *value = (tmp != 0);
            }
            else {
                in >> *value;
            }
        }
        else if constexpr (is_vector) {
            size_t count;
            in >> count;
            value->resize(count);
            for (size_t i = 0; i < count; i++) {
                if constexpr (std::is_same_v<typename T::value_type, bool>) {
                    int tmp;
                    in >> tmp;
                    (*value)[i] = (tmp != 0);
                }
                else if constexpr (std::is_same_v<typename T::value_type, std::string>) {
                    in >> (*value)[i];
                }
                else {
                    in >> (*value)[i];
                }
            }
        }
        else if constexpr(is_custom) {
            if constexpr (std::is_same_v<T, DelusiveTexture>) {
                in >> value->texturePath;
            }
            else if constexpr (std::is_same_v<T, DelusiveFont>) {
                in >> value->fontSize >> std::quoted(value->fontPath);
            }
            else if constexpr (std::is_same_v<T, DelusiveScript>) {
                in >> value->scriptName;
            }
		}
    }

    void DrawImGui() override {
        if constexpr (is_scalar) {
            if constexpr (std::is_same<T, float>::value) {
                ImGui::DragFloat(name.c_str(), value, 0.1f);
            }
            else if constexpr (std::is_same<T, int>::value) {
                ImGui::DragInt(name.c_str(), value);
            }
            else if constexpr (std::is_same<T, bool>::value) {
                ImGui::Checkbox(name.c_str(), value);
            }
            else if constexpr (std::is_same<T, glm::vec2>::value) {
                ImGui::DragFloat2(name.c_str(), glm::value_ptr(*value), 0.1f);
            }
            else if constexpr (std::is_same<T, glm::vec3>::value) {
                ImGui::DragFloat3(name.c_str(), glm::value_ptr(*value), 0.1f);
            }
            else if constexpr (std::is_same<T, glm::vec4>::value) {
                ImGui::ColorEdit4(name.c_str(), glm::value_ptr(*value));
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                char buffer[256];
                strncpy_s(buffer, value->c_str(), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = '\0';
                if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer))) {
                    *value = buffer;
                }
            }
            else {
                ImGui::Text("%s (unsupported type)", name.c_str());
            }
        }
        else if constexpr (is_vector) {
            if (ImGui::TreeNode(name.c_str())) {
                for (size_t i = 0; i < value->size(); i++) {
                    std::string label = name + "[" + std::to_string(i) + "]";
                    if constexpr (std::is_same_v<typename T::value_type, float>) {
                        ImGui::DragFloat(label.c_str(), &(*value)[i], 0.1f);
                    }
                    else if constexpr (std::is_same_v<typename T::value_type, int>) {
                        ImGui::DragInt(label.c_str(), &(*value)[i]);
                    }
                    else if constexpr (std::is_same_v<typename T::value_type, bool>) {
                        ImGui::Checkbox(label.c_str(), &(*value)[i]);
                    }
                    else if constexpr (std::is_same_v<typename T::value_type, std::string>) {
                        char buffer[256];
                        strncpy_s(buffer, (*value)[i].c_str(), sizeof(buffer));
                        buffer[sizeof(buffer) - 1] = '\0';
                        if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer))) {
                            (*value)[i] = buffer;
                        }
                    }
                }
                // Option to add/remove elements
                if (ImGui::Button(("Add " + name).c_str())) {
                    value->push_back({});
                }
                if (!value->empty()) {
                    ImGui::SameLine();
                    if (ImGui::Button(("Remove " + name).c_str())) {
                        value->pop_back();
                    }
                }
                ImGui::TreePop();
            }
        }
        else if constexpr (is_custom) {
            if constexpr (std::is_same_v<T, DelusiveTexture>) {
                ImGui::Text("Texture: %s", std::filesystem::path(value->texturePath).filename().string().c_str());
                if (ImGui::Button(("Change Texture##" + name).c_str())) {
                    ImGui::OpenPopup(("TextureBrowser##" + name).c_str());
                }
                if (ImGui::BeginPopup(("TextureBrowser##" + name).c_str())) {
                    std::function<void(const std::filesystem::path&)> DrawDirectory;
                    DrawDirectory = [&](const std::filesystem::path& path) {
                        for (const auto& entry : std::filesystem::directory_iterator(path)) {
                            if (entry.is_directory()) {
                                if (ImGui::BeginMenu((entry.path().filename().string() + "/").c_str())) {
                                    DrawDirectory(entry.path());
                                    ImGui::EndMenu();
                                }
                            }
                            else if (entry.is_regular_file()) {
                                std::string ext = entry.path().extension().string();
                                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                                    std::string filename = entry.path().filename().string();
                                    if (ImGui::Selectable(filename.c_str())) {
                                        value->texturePath = entry.path().string();
                                        ImGui::CloseCurrentPopup();
                                    }
                                }
                            }
                        }
                     };
                    DrawDirectory("assets/sprites");
                    ImGui::EndPopup();
                }
            }
            else if constexpr (std::is_same_v<T, DelusiveFont>) {
                ImGui::Text("Font: %s", std::filesystem::path(value->fontPath).filename().string().c_str());
                ImGui::DragFloat(("Size##" + name).c_str(), &value->fontSize, 1.0f, 6.0f, 128.0f);
                if (ImGui::Button(("Change Font##" + name).c_str())) {
                    ImGui::OpenPopup(("FontBrowser##" + name).c_str());
                }
                if (ImGui::BeginPopup(("FontBrowser##" + name).c_str())) {
                    for (auto& entry : std::filesystem::recursive_directory_iterator("assets/fonts")) {
                        if (entry.is_regular_file()) {
                            std::string ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                            if (ext == ".ttf" || ext == ".otf") {
                                std::string filename = entry.path().filename().string();
                                if (ImGui::Selectable(filename.c_str())) {
                                    value->fontPath = entry.path().string();
                                    ImGui::CloseCurrentPopup();
                                }
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
            }
            else if constexpr (std::is_same_v<T, DelusiveScript>) {
                auto names = ScriptRegistry::Instance().GetNames();
                int currentIndex = 0;
                for (int i = 0; i < (int)names.size(); i++) {
                    if (names[i] == value->scriptName) { currentIndex = i; break; }
                }
                if (ImGui::Combo(name.c_str(), &currentIndex,
                    [](void* data, int idx, const char** out_text) {
                        auto& vec = *reinterpret_cast<std::vector<std::string>*>(data);
                        if (idx < 0 || idx >= (int)vec.size()) return false;
                        *out_text = vec[idx].c_str();
                        return true;
                    },
                    (void*)&names, (int)names.size())) {
                    value->scriptName = names[currentIndex];
                }
            }
        }
    }
};

class PropertyRegistry {
public:
    std::vector<std::unique_ptr<PropertyBase>> properties;

    PropertyRegistry() = default;
    PropertyRegistry(const PropertyRegistry&) = delete;
    PropertyRegistry& operator=(const PropertyRegistry&) = delete;
    PropertyRegistry(PropertyRegistry&&) noexcept = default;
    PropertyRegistry& operator=(PropertyRegistry&&) noexcept = default;

    template<typename T>
    void Register(const std::string& name, T* var) {
        for (auto& prop : properties) {
            if (prop->GetName() == name) {
                // skip or log warning instead of adding duplicate
                return;
            }
        }
        properties.emplace_back(std::make_unique<Property<T>>(name, var));
    }

    void Serialize(std::ostream& out) const {
        for (auto& p : properties) {
            out << p->name << "=";
            p->Serialize(out);
            out << "\n";
        }
    }

    void Deserialize(std::istream& in) {
        std::string line;
        while (true) {
            std::streampos pos = in.tellg(); // remember position
            if (!std::getline(in, line)) break;

            if (line.empty()) continue;

            // Stop at next block header
            if (!line.empty() && line.front() == '[') {
                in.seekg(pos);
                break;
            }

            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            std::istringstream valStream(val);
            for (auto& p : properties) {
                if (p->name == key) {
                    p->Deserialize(valStream);
                    break;
                }
            }
        }
    }

    void DrawImGui() {
        for (auto& p : properties) {
            p->DrawImGui();
        }
    }
};