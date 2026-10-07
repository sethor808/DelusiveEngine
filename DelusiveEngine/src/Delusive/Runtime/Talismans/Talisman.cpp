#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>

Talisman::Talisman()
	: registry(std::make_unique<PropertyRegistry>())
{
	//No virtual calls here, so the base can register - the type is filled in at save time
	registry->category = "Talisman";
	registry->Register("id", &id);
	registry->Register("hp", &hp);
	registry->Register("isBroken", &isBroken);
}

Talisman::~Talisman() = default;

void Talisman::Serialize(DelusiveParser::DataBlock& out) const {
	registry->type = GetType();
	registry->Serialize(out);
}

void Talisman::Deserialize(DelusiveParser::DataBlock& in) {
	registry->Deserialize(in);
}

void Talisman::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
	DelusiveParser::DataBlock self;
	Serialize(self);
	self.id = id;
	out.push_back(std::move(self));
}

bool Talisman::TakeDamage() {
	hp -= 1;

	if (hp <= 0) {
		isBroken = true;
		return isBroken;
	} else {
		isBroken = false;
		return isBroken;
	}
}

std::string Talisman::GetBaseTexture() const {
	return talismanBase;
}

std::string Talisman::GetGlyphTexture() const {
	return talismanGlyph;
}

std::string Talisman::GetStringTexture() const {
	return TALISMAN_STRING;
}