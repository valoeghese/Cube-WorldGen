#include <set>

#include "CWSDK/cwsdk.h"

namespace cubewg {
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

		cube::Block* GetBlock(LongVector3 block_pos);

		void SetBlock(LongVector3 block_pos, int r, int g, int b, cube::Block::Type type, std::set<cube::Zone*>& to_remesh);
	};
}