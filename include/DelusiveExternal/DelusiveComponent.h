#pragma once
#include <string>

class DelusiveComponent {
public:
	virtual ~DelusiveComponent() = default;
	virtual std::string GetType() const = 0;
};