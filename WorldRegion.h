#pragma once

#include "CWSDK/cwsdk.h"

#include "Structure.h"

namespace cubewg {
	// consts
	const int kNoPosition = -32768;

	cube::Block BlockOf(const int r, const int g, const int b, const cube::Block::Type type = cube::Block::Solid, const bool breakable = false);

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

		/* Takes an [x, y] position and returns base z at that position. If the zone hasn't loaded, returns cubewg::kNoPosition.
		*/
		int GetBaseZ(LongVector2 block_pos);

		/* Takes an [x, y] position and returns height at that position. That is, the z-pos of the first block that is either air, water, lava, or has no block currently assigned, while counting up through blocks at the given column.
		 * If the zone has not loaded, it will return cubewg::kNoPosition. Additionally, if require_surface is enabled and it cannot find a position with solid ground beneath it, it will return cubewg::kNoPosition.
		 */
		int GetHeight(LongVector2 block_pos, const bool require_surface = false);

		cube::Block* GetBlock(LongVector3 block_pos);

		void SetBlock(LongVector3 block_pos, cube::Block block, std::set<cube::Zone*>& to_remesh);

		// static methods
		/* Add a structure to be generated in the world.
		*/
		static void AddStructure(cubewg::Structure* structure);

		/* Internal method called on initialisation.
		*/
		static void Initialise();
		/* Internal method called on zone deletion.
		*/
		static void CleanUpBuffers(IntVector2 zone_pos);
		/* Internal method called on zone generation.
		*/
		static void GenerateInZone(cube::Zone* zone, std::set<cube::Zone*>& to_remesh);
	};
}