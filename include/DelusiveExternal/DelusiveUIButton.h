#pragma once  
#include <DelusiveExternal/ScriptUIElement.h>  

class DelusiveUIButton : public ScriptUIElement {  
public:  
    //Don't think we need these  
    /*  
    void Update(float) override {}  
    void Draw(const glm::mat4& proj) override;  
    void HandleMouse(const glm::vec2& mouse, bool mouseDown) override;  
    void SetTexturePath(const std::string&);  
    */  

    void SetLabel(std::string&);
    bool SupportsClick() override { return true; }
    void SetOnClick(std::function<void()>) override;
};