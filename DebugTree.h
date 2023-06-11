#pragma once

#include "WorldRegion.h"
#include "Structure.h"

namespace cubewg {
	class DebugTree : public Structure {
	private:
		cube::Block blue_leaves;
		cube::Block log;
	public:
		DebugTree();

		int GenerateAt(WorldRegion& region, const IntVector3& origin, std::set<cube::Zone*>& to_remesh) override;
		bool Generate(WorldRegion& region, const IntVector2& zone_position, std::set<cube::Zone*>& to_remesh) override;
	};
}