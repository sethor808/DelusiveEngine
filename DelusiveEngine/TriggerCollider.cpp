#include "TriggerCollider.h"

void TriggerCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetColliderType() == ColliderType::Solid) {
		std::cout << "[Trigger] occurred by solid collider." << std::endl;
		//Call damage here
	}
}

void TriggerCollider::DrawImGui() {
    ImGui::Text("Trigger Collider");
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

void TriggerCollider::Serialize(std::ofstream& out) const {
    out << "Trigger Collider\n";
    out << name << "\n";
    out << transform.position.x << " " << transform.position.y << "\n";
    out << transform.rotation << "\n";
    out << transform.scale.x << " " << transform.scale.y << "\n";
}

void TriggerCollider::Deserialize(std::ifstream& in) {
    in >> name;
    in >> transform.position.x >> transform.position.y;
    in >> transform.rotation;
    in >> transform.scale.x >> transform.scale.y;
    in.ignore();
}