#include "DelusiveUI.h"

std::unique_ptr<UIElement> CreateUIElementByType(const std::string& type, DelusiveRenderer& renderer) {
     if (type == "UILabel") {
         return std::make_unique<UILabel>(renderer);
     }
     if (type == "UIButton") {
         return std::make_unique<UIButton>(renderer);
     }
     if (type == "UIImage") {
         return std::make_unique<UIImage>(renderer);
     }
     if (type == "UIPanel") {
         return std::make_unique<UIPanel>(renderer);
     }

     // Unknown type fallback
     return nullptr;
}