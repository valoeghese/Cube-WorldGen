#pragma once

#include "CWSDK/cwsdk.h"

namespace cubewg {
	// prevent mutual inclusion of headers
	class WorldRegion;

	class Structure {
	public:
		/* This calls for the structure to generate with a specific origin. This is called directly when the debug command /generate is run. An int return value is provided for those who wish to pass information from this if calling it from Generate.
		*/
		virtual int GenerateAt(WorldRegion& region, const IntVector3& origin, std::set<cube::Zone*>& to_remesh) = 0;
		/* This calls for the structure to place itself within a zone. Returns whether a major structure was generated within the zone.
		*/
		virtual bool Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) = 0;
	};
}