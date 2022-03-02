#include "City.h"

cubewg::City::City() {
	city_wall = BlockOf(130, 150, 160);
}

cubewg::City::~City() {
}

void cubewg::City::Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) {
	// generate city walls
	for (int x = 0; x < cube::BLOCKS_PER_ZONE; x++) {
		for (int y = 0; y < cube::BLOCKS_PER_ZONE; y++) {
			double worley = cities_grid.Worley((zone_position.x * cube::BLOCKS_PER_ZONE + x) * 0.001, (zone_position.y * cube::BLOCKS_PER_ZONE + y) * 0.001);

			if (worley <= 0.1 && worley >= 0.08) {
				int height = region.GetHeight(LongVector2(x, y), true);

				if (height != kNoPosition) {
					for (int zo = 0; zo < 10; zo++) {
						region.SetBlock(LongVector3(x, y, height + zo), city_wall, to_remesh);
					}
				}
			}
		}
	}
}
