#include "City.h"

#include <cmath>

const int kCityGridScale = 2000;

const double kCityBorderRadius = 282;
const double kCityWallRadius = kCityBorderRadius + 36;
const double kCityShapeRadius = kCityWallRadius + 100;

#define SQR_CITY_BORDER_RADIUS (kCityBorderRadius * kCityBorderRadius)
#define SQR_CITY_WALL_RADIUS   (kCityWallRadius * kCityWallRadius)
#define SQR_CITY_SHAPE_RADIUS  (kCityShapeRadius * kCityShapeRadius)

cubewg::City::City() : cities_grid(JitteredGrid(0, 0.2, kCityGridScale)) {
	city_wall = BlockOf(130, 150, 160);
	pavement = BlockOf(90, 90, 90, cube::Block::Ground);
	air = BlockOf(0, 0, 0, cube::Block::Air);
}

cubewg::City::~City() {
}

int cubewg::City::GenerateAt(WorldRegion& region, const IntVector3& origin, std::set<cube::Zone*>& to_remesh) {
	JitteredPoint pos = cities_grid.FindNearestPoint(origin.x, origin.y);

	cube::GetGame()->PrintMessage(L"City Data:\n");

	std::wstring centre_msg = L"- Centre: (" + std::to_wstring(pos.x * cube::DOTS_PER_BLOCK) + L", " + std::to_wstring(pos.y * cube::DOTS_PER_BLOCK) + L")\n";
	cube::GetGame()->PrintMessage(centre_msg.c_str());

	long double dx = pos.x - origin.x;
	long double dy = pos.y - origin.y;
	std::wstring dist_msg = L"- Horizontal Distance: " + std::to_wstring(std::sqrt(dx * dx + dy * dy)) + L"\n";
	cube::GetGame()->PrintMessage(dist_msg.c_str());
	return 0;
}

bool cubewg::City::Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) {
	bool generated = false;

	// flatten terrain
	JitteredPoint center = cities_grid.FindNearestPoint(zone_position.x * cube::BLOCKS_PER_ZONE, zone_position.y * cube::BLOCKS_PER_ZONE);
	int flattened_height = (int) cube::GetGame()->world->GetZoneStructureHeight(center.x / cube::BLOCKS_PER_ZONE, center.y / cube::BLOCKS_PER_ZONE);

	cube::Zone* zone = region.GetZone({ zone_position.x * cube::BLOCKS_PER_ZONE, zone_position.y * cube::BLOCKS_PER_ZONE });

	for (int x = 0; x < cube::BLOCKS_PER_ZONE; x++) {
		for (int y = 0; y < cube::BLOCKS_PER_ZONE; y++) {
			double sqr_dist_2_city_centre = cities_grid.SqrDist2Nearest((zone_position.x * cube::BLOCKS_PER_ZONE + x), (zone_position.y * cube::BLOCKS_PER_ZONE + y));

			if (sqr_dist_2_city_centre < SQR_CITY_WALL_RADIUS) {
				int field_index = x * cube::BLOCKS_PER_ZONE + y;
				cube::Field* field = &zone->fields[field_index];
				field->base_z = flattened_height;
			} else if (sqr_dist_2_city_centre < SQR_CITY_SHAPE_RADIUS) {
				int field_index = x * cube::BLOCKS_PER_ZONE + y;
				cube::Field* field = &zone->fields[field_index];

				float prog = sqrtf((sqr_dist_2_city_centre - SQR_CITY_WALL_RADIUS) / (SQR_CITY_SHAPE_RADIUS - SQR_CITY_WALL_RADIUS));
				int interpolated_height = (int)(flattened_height + prog * (field->base_z - flattened_height));
				field->base_z = interpolated_height;
			}

			// remove junk
			if (sqr_dist_2_city_centre < SQR_CITY_WALL_RADIUS) {
				int field_index = x * cube::BLOCKS_PER_ZONE + y;
				cube::Field* field = &zone->fields[field_index];
				field->blocks.clear();
			}
		}
	}

	// generate city walls and pavement
	for (int x = 0; x < cube::BLOCKS_PER_ZONE; x++) {
		for (int y = 0; y < cube::BLOCKS_PER_ZONE; y++) {
			double sqr_dist_2_city_centre = cities_grid.SqrDist2Nearest((zone_position.x * cube::BLOCKS_PER_ZONE + x), (zone_position.y * cube::BLOCKS_PER_ZONE + y));

			if (sqr_dist_2_city_centre <= SQR_CITY_WALL_RADIUS && sqr_dist_2_city_centre >= SQR_CITY_BORDER_RADIUS) {
				generated = true;
				int height = region.GetBaseZ(LongVector2(x, y));//region.GetHeight(LongVector2(x, y), Heightmap::WORLD_SURFACE) + 1;
				const int wall_height = (sqr_dist_2_city_centre <= 314 * 314 && sqr_dist_2_city_centre >= 284 * 284) ? 11 : 14;

				if (height != kNoPosition) {
					// TODO account for lava and trees
					for (int zo = 0; zo < wall_height; zo++) {
						region.SetBlock(LongVector3(x, y, height + zo), city_wall, to_remesh);
					}
				}
			}
			else if (sqr_dist_2_city_centre < SQR_CITY_BORDER_RADIUS) {
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
		int height = region.GetBaseZ(LongVector2(32, 32));//region.GetHeight(LongVector2(32, 32), Heightmap::WORLD_SURFACE) + 1;

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
