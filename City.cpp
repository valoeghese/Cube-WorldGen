#include "City.h"

#include <cmath>

const int kCityGridScale = 2000;

cubewg::City::City() : cities_grid(JitteredGrid(0, 0.2, kCityGridScale)) {
	city_wall = BlockOf(130, 150, 160);
	pavement = BlockOf(90, 90, 90, cube::Block::Ground);
}

cubewg::City::~City() {
}

int cubewg::City::GenerateAt(WorldRegion& region, const IntVector3& origin, std::set<cube::Zone*>& to_remesh) {
	JitteredPoint pos = cities_grid.FindNearestPoint(origin.x, origin.y);

	cube::GetGame()->PrintMessage(L"City Data:\n");

	std::wstring centre_msg = L"- Centre: (" + std::to_wstring(pos.x) + L", " + std::to_wstring(pos.y) + L")\n";
	cube::GetGame()->PrintMessage(centre_msg.c_str());

	long double dx = pos.x - origin.x;
	long double dy = pos.y - origin.y;
	std::wstring dist_msg = L"- Horizontal Distance:\n" + std::to_wstring(std::sqrt(dx * dx + dy * dy));
	cube::GetGame()->PrintMessage(dist_msg.c_str());
	return 0;
}

bool cubewg::City::Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) {
	bool generated = false;

	// generate city walls and pavement
	for (int x = 0; x < cube::BLOCKS_PER_ZONE; x++) {
		for (int y = 0; y < cube::BLOCKS_PER_ZONE; y++) {
			double sqr_dist_2_city_centre = cities_grid.SqrDist2Nearest((zone_position.x * cube::BLOCKS_PER_ZONE + x), (zone_position.y * cube::BLOCKS_PER_ZONE + y));

			if (sqr_dist_2_city_centre <= 316 * 316 && sqr_dist_2_city_centre >= 282 * 282) {
				generated = true;
				int height = region.GetHeight(LongVector2(x, y), Heightmap::WORLD_SURFACE) + 1;
				const int wall_height = (sqr_dist_2_city_centre <= 314 * 314 && sqr_dist_2_city_centre >= 284 * 284) ? 11 : 14;

				if (height != kNoPosition) {
					// TODO account for lava and trees
					for (int zo = 0; zo < wall_height; zo++) {
						region.SetBlock(LongVector3(x, y, height + zo), city_wall, to_remesh);
					}
				}
			}
			else if (sqr_dist_2_city_centre < 282 * 282) {
				generated = true;
				int height = region.GetHeight(LongVector2(x, y), Heightmap::WORLD_SURFACE) + 1;

				if (height != kNoPosition) {
					region.SetBlock(LongVector3(x, y, height - 1), pavement, to_remesh);
				}
			}
		}
	}

	// generate buildings
	double zone_centre_dist_to_city_centre = cities_grid.SqrDist2Nearest((zone_position.x * cube::BLOCKS_PER_ZONE + 32), (zone_position.y * cube::BLOCKS_PER_ZONE + 32));

	if (zone_centre_dist_to_city_centre < 269 * 269) {
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
