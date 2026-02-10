#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <functional>

class UUID {
public:
    UUID() = default; //Don't generate on its own

	static UUID GenerateRandom() {
		UUID id;
		id.Generate();
		return id;
	}

    bool IsValid() const {
        return high != 0 || low != 0;
    }

    void Reset() {
        high = 0;
        low = 0;
    }

	std::string ToString() const {
		std::ostringstream ss;
		ss << std::hex << std::setfill('0')
			<< std::setw(16) << high << "-"
			<< std::setw(16) << low;
		return ss.str();
	}

	bool operator==(const UUID& other) const noexcept {
		return high == other.high && low == other.low;
	}

	bool operator!=(const UUID& other) const noexcept {
		return !(*this == other);
	}

	struct Hash {
		std::size_t operator()(const UUID& id) const noexcept {
			// Mix the two 64-bit halves
			return std::hash<uint64_t>()(id.high) ^ (std::hash<uint64_t>()(id.low) << 1);
		}
	};

	void FromString(const std::string& str) {
		std::istringstream ss(str);
		std::string highStr, lowStr;
		if (std::getline(ss, highStr, '-') && std::getline(ss, lowStr)) {
			high = std::stoull(highStr, nullptr, 16);
			low = std::stoull(lowStr, nullptr, 16);
		}
	}

private:
	uint64_t high = 0;
	uint64_t low = 0;

	void Generate() {
		static thread_local std::mt19937_64 gen(std::random_device{}());
		static std::uniform_int_distribution<uint64_t> dis;

		high = dis(gen);
		low = dis(gen);
	}

	
};