#include "DelusiveUI.h"

std::unique_ptr<UIElement> CreateUIElementByType(const std::string& type) {
     if (type == "UILabel") {
         return std::make_unique<UILabel>();
     }
     if (type == "UIButton") {
         return std::make_unique<UIButton>();
     }
     if (type == "UIImage") {
         return std::make_unique<UIImage>();
     }
     if (type == "UIPanel") {
         return std::make_unique<UIPanel>();
     }

     // Unknown type fallback
     return nullptr;
}