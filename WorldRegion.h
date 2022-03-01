#pragma once

#include "CWSDK/cwsdk.h"

namespace cubewg {
	// consts
	const int kNoPosition = -32768;

	/* Abstraction between zones and worlds with some additional useful utilities. Zonal world generation hooks into zone buffers.
	*/
	class WorldRegion {
	private:
		cube::World* world;
		cube::Zone* zone;

	public:
		/* Generation for runtime/tests using world.
		*/
		WorldRegion(cube::World* world) {
			this->world = world;
			this->zone = nullptr;
		}

		/* Generation for worldgen using zone/buffers
		*/
		WorldRegion(cube::Zone* zone) {
			this->world = nullptr;
			this->zone = zone;
		}

		/*
		 * Takes an [x, y] position and returns height at that position. That is, the z-pos of the first block that is either air, water, lava, or has no block currently assigned, while counting up through blocks at the given column.
		 * If the zone has not loaded, it will return cubewg::kNoPosition. Additionally, if require_surface is enabled and it cannot find a position with solid ground beneath it, it will return cubewg::kNoPosition.
		 */
		int GetHeight(LongVector2 block_pos, const bool require_surface = false);

		cube::Block* GetBlock(LongVector3 block_pos);

		void SetBlock(LongVector3 block_pos, cube::Block block, std::set<cube::Zone*>& to_remesh);

		// static methods
		static void Initialise();
		static void CleanUpBuffers(IntVector2 zone_pos);
		static void PasteZone(cube::Zone* zone, std::set<cube::Zone*>& to_remesh);
	};
}