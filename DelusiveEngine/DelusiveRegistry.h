#pragma once
#include <string>
#include <functional>
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

    virtual void Serialize(std::ostream& out) const = 0;
    virtual void Deserialize(std::istream& in) = 0;
    virtual void DrawImGui() = 0;
};

template<typename T>
class Property : public PropertyBase {
    static_assert(
        std::is_same_v<T, float> ||
        std::is_same_v<T, int> ||
        std::is_same_v<T, bool> ||             
        std::is_same_v<T, glm::vec2> ||
        std::is_same_v<T, glm::vec3> ||
        std::is_same_v<T, std::string>,
        "Property<T>: Unsupported type"
        );

    T* value;
public:
    Property(const std::string& n, T* val) : value(val) {
        name = n;
    }

    void Serialize(std::ostream& out) const override {
        if constexpr (std::is_same<T, glm::vec2>::value) {
            out << value->x << " " << value->y;
        }
        else if constexpr (std::is_same<T, glm::vec3>::value) {
            out << value->x << " " << value->y << " " << value->z;
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

    void Deserialize(std::istream& in) override {
        if constexpr (std::is_same<T, glm::vec2>::value) {
            in >> value->x >> value->y;
        }
        else if constexpr (std::is_same<T, glm::vec3>::value) {
            in >> value->x >> value->y >> value->z;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            std::getline(in, *value);
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

    void DrawImGui() override {
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
        while (std::getline(in, line)) {
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