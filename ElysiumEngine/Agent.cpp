#include "Agent.h"

Agent::Agent() {

}

Agent::~Agent() {

}

void Agent::SetPosition(const glm::vec2& pos) {
	transform.position = pos;
}

void Agent::SetRotation(const float rotation) {
	transform.rotation = rotation;
}

void Agent::SetScale(const glm::vec2 scale) {
	transform.scale = scale;
}

TransformComponent& Agent::GetTransform() {
	return transform;
}

const TransformComponent& Agent::GetTransform() const {
	return transform;
}

void Agent::SaveToFile(std::ofstream& out) const {
	out << name << "\n";
	out << transform.position.x << " " << transform.position.y << "\n";
	out << transform.rotation << "\n";
	out << transform.scale.x << " " << transform.scale.y << "\n";

	//TODO: Save components
}

void Agent::LoadFromFile(std::ifstream& in) {
	std::getline(in, name);
	in >> transform.position.x >> transform.position.y;
	in >> transform.rotation;
	in >> transform.scale.x >> transform.scale.y;
	in.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip to next line
	
	// TODO: Load colliders
}