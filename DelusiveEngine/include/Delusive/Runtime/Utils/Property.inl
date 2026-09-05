#pragma once
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Core/IDLink.h>
#include <Delusive/Runtime/Utils/UUID.h>
#include <Delusive/Runtime/Utils/PropertyDraw.h>
#include <type_traits>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <iostream>
#include <iomanip>

//==== [ZONE: link trait — matches any DelusiveLink<T>] ====
template<typename>   struct is_delusive_link : std::false_type {};
template<typename U> struct is_delusive_link<DelusiveLink<U>> : std::true_type {};
template<typename T> inline constexpr bool is_delusive_link_v = is_delusive_link<T>::value;
//==== [/ZONE: link trait] ====

//==== [ZONE: link trait — matches any DelusiveLink<T>] ====
template<typename>   struct is_delusive_object : std::false_type {};
template<typename U> struct is_delusive_object<DelusiveObject<U>> : std::true_type {};
template<typename T> inline constexpr bool is_delusive_object_v = is_delusive_object<T>::value;
//==== [/ZONE: link trait] ====


#pragma region Property declaration

// Property<T> Template
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
        std::is_same_v<T, UUID> ||
        is_delusive_link_v<T> || //[CLAUDE: trait, DelusiveLink is templated]
        is_delusive_object_v<T> ||
        std::is_same_v<T, DelusiveTexture> ||
        std::is_same_v<T, DelusiveFont> ||
        std::is_same_v<T, DelusiveUILink>;
    static_assert(
        is_scalar || is_vector || is_custom,
        "Property<T>: Unsupported type"
        );

    T* value;

#pragma endregion

public:
    Property(const std::string& n, T* val) : value(val) {
        name = n;
    }

#pragma region Serialize
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
                out << std::quoted(*value);
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
                else if constexpr (std::is_same_v<typename T::value_type, std::string>) {
                    out << std::quoted((*value)[i]);
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
            else if constexpr (std::is_same_v<T, UUID>) {
                out << value->ToString();
            }
            else if constexpr (is_delusive_link_v<T>) { //[CLAUDE: trait swap]
                out << value->id.ToString();
            }
        }
    }
#pragma endregion

#pragma region Deserialize
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
                if (in.peek() == '"') {
                    in >> std::quoted(*value);
                }
                else {
                    in >> *value; // backward compatibility
                }
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
                    in >> std::quoted((*value)[i]);
                }
                else {
                    in >> (*value)[i];
                }
            }
        }
        else if constexpr (is_custom) {
            if constexpr (std::is_same_v<T, DelusiveTexture>) {
                in >> value->texturePath;
            }
            else if constexpr (std::is_same_v<T, DelusiveFont>) {
                in >> value->fontSize >> std::quoted(value->fontPath);
            }
            else if constexpr (std::is_same_v<T, UUID>) {
                std::string uuidStr;
                in >> uuidStr;
                value->FromString(uuidStr);
			}
            else if constexpr (is_delusive_link_v<T>) { //[CLAUDE: trait swap]
                std::string uuidStr;
                in >> uuidStr;

                value->id.FromString(uuidStr);
            }
        }
    }

#pragma endregion

#pragma region DrawImGui
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
                // Add/remove buttons
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
            //==== [ZONE: custom draws — bodies in PropertyDraw.cpp] ====
            if constexpr (std::is_same_v<T, DelusiveTexture>) {
                DrawTextureUI(*value, name);
            }
            else if constexpr (std::is_same_v<T, DelusiveFont>) {
                DrawFontUI(*value, name);
            }
            else if constexpr (is_delusive_link_v<T>) {
                DrawLinkUI(*value, name);
            }
            else if constexpr (std::is_same_v<T, DelusiveUILink>) {
                DrawUILinkUI(*value, name);
            }
            else if constexpr (std::is_same_v<T, UUID>) {
                ImGui::Text("UUID: %s", value->ToString().c_str());
            }
            //==== [/ZONE: custom draws] ====
        }
    }
};
#pragma endregion

// Registry::Register<T>
template<typename T>
void PropertyRegistry::Register(const std::string& name, T* var) {
    for (auto& prop : properties) {
        if (prop->GetName() == name) {
            std::cerr << "[PropertyRegistry] Attempt to duplicate value: " << name << std::endl;
            return;
        }
    }
    
    auto newProp = std::make_unique<Property<T>>(name, var);
    properties.emplace_back(std::move(newProp)); //[CLAUDE: unique_ptr must move]
}