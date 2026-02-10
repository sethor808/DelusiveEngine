#include <Delusive/Runtime/UI/UITalismanButton.h>
#include <Delusive/Runtime/Talismans/DelusiveTalismans.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>

UITalismanButton::UITalismanButton(DelusiveRenderer& renderer)
	: UIButton(renderer) 
{
	name = "New UITalismanButton";
}

std::unique_ptr<UIElement> UITalismanButton::Clone() const {
    auto copy = std::make_unique<UITalismanButton>(renderer);

    copy->SetPosition(position);
    copy->SetSize(size);
    copy->SetName(name);
    copy->SetEnabled(enabled);

    return copy;
}

void UITalismanButton::RegisterProperties() {
    UIElement::RegisterProperties();
}

void UITalismanButton::HandleMouse(const glm::vec2& mousePos, bool isPressed) {
    if (!talisman) return;
    
    UIButton::HandleMouse(mousePos, isPressed);
}

void UITalismanButton::BindTalisman(Talisman* t) {
	talisman = t;
}

void UITalismanButton::Draw(const glm::mat4& projection) {
	if (!talisman) return;
	
	UIButton::Draw(projection);

    GLuint tex = renderer.GetTexture(talisman->GetBaseTexture());
    if (!tex)
        return;

    glm::vec2 worldPos = position * DELUSIVE_PIXEL_SCALE;
    glm::vec2 worldSize = size * DELUSIVE_PIXEL_SCALE;

    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(worldPos, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(worldSize, 1.0f));

    renderer.Submit({
        .modelMatrix = model,
        .color = glm::vec4(1.0f),
        .textureID = tex,
        .layer = 200,
        .isUI = true
        });
}