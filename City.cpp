#include "City.h"

cubewg::City::City() {
	city_wall = BlockOf(130, 150, 160);
	pavement = BlockOf(90, 90, 90, cube::Block::Ground);
	cities_grid = JitteredGrid(0, 0.2);
}

cubewg::City::~City() {
}

int cubewg::City::GenerateAt(WorldRegion& region, const IntVector3& origin, std::set<cube::Zone*>& to_remesh) {
	cube::GetGame()->PrintMessage(L"City Structure does not support runtime generation as it too complex.");
	return 0;
}

bool cubewg::City::Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) {
	bool generated = false;

	// generate city walls
	for (int x = 0; x < cube::BLOCKS_PER_ZONE; x++) {
		for (int y = 0; y < cube::BLOCKS_PER_ZONE; y++) {
			double worley = cities_grid.Worley((zone_position.x * cube::BLOCKS_PER_ZONE + x) * 0.0005, (zone_position.y * cube::BLOCKS_PER_ZONE + y) * 0.0005);

			if (worley <= 0.025 && worley >= 0.02) {
				generated = true;
				int height = region.GetHeight(LongVector2(x, y), Heightmap::WORLD_SURFACE) + 1;
				const int wall_height = (worley <= 0.024 && worley >= 0.021) ? 11 : 14;

				if (height != kNoPosition) {
					// TODO account for lava and trees
					for (int zo = 0; zo < wall_height; zo++) {
						region.SetBlock(LongVector3(x, y, height + zo), city_wall, to_remesh);
					}
				}
			}
			else if (worley < 0.02) {
				generated = true;
				int height = region.GetHeight(LongVector2(x, y), Heightmap::WORLD_SURFACE) + 1;

				if (height != kNoPosition) {
					region.SetBlock(LongVector3(x, y, height - 1), pavement, to_remesh);
				}
			}
		}
	}

	// generate buildings
	double centre_worley = cities_grid.Worley((zone_position.x * cube::BLOCKS_PER_ZONE + 32) * 0.0005, (zone_position.y * cube::BLOCKS_PER_ZONE + 32) * 0.0005);

	if (centre_worley < 0.018) {
		generated = true;
		int height = region.GetHeight(LongVector2(32, 32), Heightmap::WORLD_SURFACE) + 1;

		cube::Block* abv_surface_block = region.GetBlock(LongVector3(32, 32, height));
		
		if (abv_surface_block) {
			for (int xo = -10; xo <= 10; xo++) {
				int local_x = xo + 32;

				for (int yo = -10; yo <= 10; yo++) {
					int local_y = yo + 32;

					if (std::abs(xo) == 10 || std::abs(yo) == 10) { // only on borders
						for (int zo = 0; zo <= 10; zo++) {
							region.SetBlock(LongVector3(local_x, local_y, zo + height), city_wall, to_remesh);
						}
					}
				}
			}
		}
	}

	return generated;
}
