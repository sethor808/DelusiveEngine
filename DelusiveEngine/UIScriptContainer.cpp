#include "UIScriptContainer.h"

UIScriptContainer::UIScriptContainer(DelusiveRenderer& renderer) 
	: UIElement(renderer)
{

}

std::unique_ptr<UIElement> UIScriptContainer::Clone() const{
	return std::make_unique<UIScriptContainer>(renderer);
}