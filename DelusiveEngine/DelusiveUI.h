#pragma once
#include "DelusiveRenderer.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIPanel.h"

std::unique_ptr<UIElement> CreateUIElementByType(const std::string&, DelusiveRenderer&);