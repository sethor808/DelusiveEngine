#include <Delusive/Runtime/Agents/EnvironmentAgent.h>

EnvironmentAgent::EnvironmentAgent(DelusiveInstance& instance)
    : Agent(instance)
{
    SetName("New EnvironmentAgent");
	SetScale({ 1.0f, 1.0f });
}

std::string EnvironmentAgent::GetType() const{
    return "EnvironmentAgent";
}

std::unique_ptr<Agent> EnvironmentAgent::Clone(Scene* scene) const {
    auto copy = std::make_unique<EnvironmentAgent>(instance);
    copy->name = GetName();
    CloneBaseProperties(copy.get(), scene);
    return copy;
}

void EnvironmentAgent::Update(float deltaTime) {

}

void EnvironmentAgent::Draw(const glm::mat4& projection) const {
    for (const auto& comp : this->GetComponents()) {
        comp->Draw(projection);
    }
}