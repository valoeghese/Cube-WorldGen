#include <set>
#include <optional>

#include "CWSDK/cwsdk.h"

namespace cubewg {
	const int kNoPosition = -32768;

	class WorldRegion {
	private:
		cube::World* world;
		cube::Zone* zone;

	public:
		WorldRegion(cube::World* world) {
			this->world = world;
			this->zone = nullptr;
		}

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

		void SetBlock(LongVector3 block_pos, int r, int g, int b, cube::Block::Type type, std::set<cube::Zone*>& to_remesh);
	};
}