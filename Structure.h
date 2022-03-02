#pragma once

#include "CWSDK/cwsdk.h"

namespace cubewg {
	// prevent mutual inclusion of headers
	class WorldRegion;

	class Structure {
	public:
		virtual void Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) = 0;
	};
}