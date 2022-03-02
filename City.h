#pragma once

#include "WorldRegion.h"
#include "Structure.h"
#include "JitteredGrid.h"

namespace cubewg {
	class City : public Structure {
	private:
		JitteredGrid cities_grid;
		cube::Block city_wall;
	public:
		City();
		~City();

		void Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh);
	};
}