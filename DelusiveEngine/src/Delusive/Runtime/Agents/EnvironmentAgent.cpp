#include <Delusive/Runtime/Agents/EnvironmentAgent.h>

EnvironmentAgent::EnvironmentAgent(DelusiveInstance& instance)
    : Agent(instance)
{
    SetName("New EnvironmentAgent");
	SetScale({ 1.0f, 1.0f });

    RegisterProperties();
}

std::string EnvironmentAgent::GetType() const{
    return "EnvironmentAgent";
}

void EnvironmentAgent::Update(float deltaTime) {

}

void EnvironmentAgent::Draw(const glm::mat4& projection) const {
    for (const auto& comp : this->GetComponents()) {
        comp->Draw(projection);
    }
}