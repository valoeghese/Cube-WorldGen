#include "WorldRegion.h"

namespace cubewg {
	IntVector3 ToLocalBlockPos(LongVector3 block_pos) {
		return IntVector3(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z); // modulo x and y by blocks in each zone (64x64)
	}

	cube::Block* WorldRegion::GetBlock(LongVector3 block_pos) {
		if (this->world) {
			return this->world->GetBlock(block_pos);
		} else {
			return this->zone->GetBlock(ToLocalBlockPos(block_pos));
		}
	}

	void WorldRegion::SetBlock(LongVector3 block_pos, int r, int g, int b, cube::Block::Type type, std::set<cube::Zone*>& to_remesh) {
		cube::Block block;
		block.red = r;
		block.green = g;
		block.blue = b;
		block.type = cube::Block::Solid;
		block.breakable = false;

		if (this->world) {
			this->world->SetBlock(block_pos, block, false);

			IntVector2 zonePos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
			to_remesh.insert(this->world->GetZone(zonePos));

			LongVector3 blockInZonePos = LongVector3(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);

			if (blockInZonePos.x == 0) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zonePos.x - 1, zonePos.y));
				if (zone) to_remesh.insert(zone);
			} else if (blockInZonePos.x == cube::BLOCKS_PER_ZONE - 1) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zonePos.x + 1, zonePos.y));
				if (zone) to_remesh.insert(zone);
			}

			if (blockInZonePos.y == 0) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zonePos.x, zonePos.y - 1));
				if (zone) to_remesh.insert(zone);
			} else if (blockInZonePos.y == cube::BLOCKS_PER_ZONE - 1) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zonePos.x, zonePos.y + 1));
				if (zone) to_remesh.insert(zone);
			}
		}
		else {
			this->zone->SetBlock(ToLocalBlockPos(block_pos), block, false);
			to_remesh.insert(this->zone);
		}
	}
}
