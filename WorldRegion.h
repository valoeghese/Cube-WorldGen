#pragma once

#include "CWSDK/cwsdk.h"

#include "Structure.h"

namespace cubewg {
	// consts
	const int kNoPosition = -32768;
	const int kUnused = -32767;

	cube::Block BlockOf(const int r, const int g, const int b, const cube::Block::Type type = cube::Block::Solid, const bool breakable = false);

	enum class Heightmap {
		MOTION_BLOCKING,
		WORLD_SURFACE,
		OCEAN_FLOOR
	};

	/* Abstraction between zones and worlds with some additional useful utilities. Zonal world generation hooks into zone buffers.
	*/
	class WorldRegion {
	private:
		cube::World* world;
		cube::Zone* zone;
		int* heightmap_world_surface;
		int* heightmap_motion_blocking;
		int* heightmap_ocean_floor;

	public:
		/* Generation for runtime/tests using world.
		*/
		WorldRegion(cube::World* world);

		/* Generation for worldgen using zone/buffers
		*/
		WorldRegion(cube::Zone* zone);
		~WorldRegion();

		/* Takes an [x, y] position and returns base z at that position. If the zone hasn't loaded, returns cubewg::kNoPosition.
		*/
		int GetBaseZ(LongVector2 block_pos);

		/* Takes an [x, y] position and returns height at that position. The exact method is determined by the heightmap.
		 * If the zone has not loaded, it will return cubewg::kNoPosition. Additionally, if there is no valid position for the heightmap, it will return cubewg::kNoPosition.
		 */
		int GetHeight(LongVector2 block_pos, const Heightmap heightmap);

		cube::Block* GetBlock(LongVector3 block_pos);

		void SetBlock(LongVector3 block_pos, cube::Block block, std::set<cube::Zone*>& to_remesh);

		// static methods
		/* Add a structure to be generated in the world.
		*/
		static void AddStructure(std::wstring id, cubewg::Structure* structure);

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