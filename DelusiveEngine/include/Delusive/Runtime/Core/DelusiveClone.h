#pragma once
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

//One load path and one clone for every saveable type. A type plugs in by having:
//  - a DelusiveFactory<T> specialization (type string -> new object)
//  - Deserialize(DataBlock&) and SetID(UUID)                  (load)
//  - CollectBlocks(std::vector<DataBlock>&) const, root first (save)
//Agents, components, UI elements, canvases, scene systems and scripts all do.

//Builds an object from its recipe exactly as every loader does: factory by type, an
//optional setup step that must happen before loading (an agent links its scene, a
//component its owner), then deserialize and keep the recipe's id.
template<typename T>
std::unique_ptr<T> DelusiveBuild(const DelusiveParser::DataBlock& recipe, DelusiveInstance& instance,
	const std::function<void(T&)>& beforeLoad = {})
{
	std::unique_ptr<T> built = DelusiveFactory<T>::Create(recipe.type, instance);
	if (!built) {
		std::cerr << "[DelusiveBuild] Unknown " << recipe.category << " type: " << recipe.type << std::endl;
		return nullptr;
	}

	if (beforeLoad) beforeLoad(*built);
	built->Deserialize(const_cast<DelusiveParser::DataBlock&>(recipe));
	built->SetID(recipe.id);
	return built;
}

namespace DelusiveCloneDetail {
	//Save to blocks in memory, optionally re-identify them, then load a new object.
	//Owned objects resolve from those blocks through a library overlay, so the copy
	//carries unsaved edits and nothing touches disk.
	template<typename T>
	std::unique_ptr<T> Copy(const T& source, DelusiveInstance& instance, bool freshIDs,
		const std::function<void(T&)>& beforeLoad)
	{
		std::vector<DelusiveParser::DataBlock> blocks;
		source.CollectBlocks(blocks);
		if (blocks.empty()) return nullptr;

		if (freshIDs) {
			DelusiveLibrary::IDRemap remap;
			for (const auto& block : blocks) {
				if (block.id.IsValid()) remap[block.id] = UUID::GenerateRandom();
			}
			DelusiveLibrary::ApplyRemap(blocks, remap);

			//An object that never had an id (an agent not yet in a scene) still gets one
			if (!blocks.front().id.IsValid()) blocks.front().id = UUID::GenerateRandom();
		}

		DelusiveLibrary::Overlay overlay(instance.delusiveLibrary, blocks);
		return DelusiveBuild<T>(blocks.front(), instance, beforeLoad);
	}
}

//A mirror: same ids throughout, so anything referring to the original by UUID finds the
//copy instead. Play mode builds its scene this way.
template<typename T>
std::unique_ptr<T> DelusiveClone(const T& source, DelusiveInstance& instance,
	const std::function<void(T&)>& beforeLoad = {})
{
	return DelusiveCloneDetail::Copy<T>(source, instance, false, beforeLoad);
}

//An independent copy: every id inside it is fresh, and references within the copy follow
//(a copied element's children, a script's link to a sibling). Use when several copies live
//side by side - repeat container items, spawning from a prefab.
template<typename T>
std::unique_ptr<T> DelusiveInstantiate(const T& source, DelusiveInstance& instance,
	const std::function<void(T&)>& beforeLoad = {})
{
	return DelusiveCloneDetail::Copy<T>(source, instance, true, beforeLoad);
}
