#pragma once
#include "DelusiveRenderer.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIPanel.h"
#include "UITalismanDisplay.h"
#include "UITalismanButton.h"
#include "UIEquipScreen.h"
#include "UIRepeatContainer.h"

std::unique_ptr<UIElement> CreateUIElementByType(const std::string&, DelusiveRenderer&);