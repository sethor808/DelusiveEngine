#include "SolidCollider.h"

void SolidCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	std::cout << "[Solid] collided with another object." << std::endl;
}

void SolidCollider::DrawImGui() {
    ImGui::Text("Solid Collider");
    ImGui::Separator();

    ImGui::Text("Name");
    ImGui::SameLine();

    char nameBuffer[64];

    // Copy current name into buffer
    strncpy_s(nameBuffer, GetName(), sizeof(nameBuffer));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("##colliderName", nameBuffer, sizeof(nameBuffer), flags)) {
        // This triggers only when Enter is pressed
        if (nameBuffer[0] == '\0') {
            strncpy_s(nameBuffer, GetName(), sizeof(nameBuffer));
        }
        else {
            this->SetName(nameBuffer);
        }
    }

    ImGui::Separator();
    ImGui::Text("Transform");
    ImGui::Text("Position: ");
    ImGui::SameLine();
    glm::vec2 pos = transform.position;
    if (ImGui::DragFloat2("##position", glm::value_ptr(pos), 1.0f)) {
        transform.position = pos;
    }

    ImGui::Text("Rotation: ");
    ImGui::SameLine();
    float rot = transform.rotation;
    if (ImGui::DragFloat("##rotation", &rot, 0.1f)) {
        transform.rotation = rot;
    }

    ImGui::Text("Scale:    ");
    ImGui::SameLine();
    glm::vec2 scale = transform.scale;
    if (ImGui::DragFloat2("##scale", glm::value_ptr(scale), 0.1f)) {
        transform.scale = scale;
    }

    const char* shapeOptions[] = { "Box", "Circle", "Line" };
    int current = static_cast<int>(shape);

    if (ImGui::Combo("Shape", &current, shapeOptions, IM_ARRAYSIZE(shapeOptions))) {
        shape = static_cast<ShapeType>(current);
    }

    if (ImGui::Button("Delete Component")) {
        MarkToDelete();
    }
}

void SolidCollider::Serialize(std::ofstream& out) const {
    out << "enabled " << (enabled ? 1 : 0) << "\n";
    out << "transform "
        << transform.position.x << " " << transform.position.y << " "
        << transform.rotation << " "
        << transform.scale.x << " " << transform.scale.y << "\n";
    out << "shape " << static_cast<int>(shape) << "\n";
    out << "---\n";
}

void SolidCollider::Deserialize(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line == "---") break;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "enabled") {
            int state;
            iss >> state;
            enabled = (state != 0);
        }
        else if (token == "transform") {
            iss >> transform.position.x >> transform.position.y;
            iss >> transform.rotation;
            iss >> transform.scale.x >> transform.scale.y;
        }
        else if (token == "shape") {
            int shapeVal; iss >> shapeVal;
            shape = static_cast<ShapeType>(shapeVal);
        }
        else {
            break;
        }
    }
}