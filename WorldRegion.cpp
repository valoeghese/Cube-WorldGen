#include "WorldRegion.h"

namespace cubewg {
	// helper methods
	// modulo x and y by blocks in each zone (64x64)

	IntVector3 ToLocalBlockPos(LongVector3 block_pos) {
		return IntVector3(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);
	}

	IntVector2 ToLocalBlockPos(LongVector2 block_pos) {
		return IntVector2(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE));
	}

	// instance methods for WorldRegion

	cube::Block* WorldRegion::GetBlock(LongVector3 block_pos) {
		if (this->world) {
			return this->world->GetBlock(block_pos);
		} else {
			return this->zone->GetBlock(ToLocalBlockPos(block_pos));
		}
	}

	int WorldRegion::GetHeight(LongVector2 block_pos, const bool require_surface) {
		cube::Zone* zone;
		IntVector2 local_block_pos = ToLocalBlockPos(block_pos);

		if (this->world) {
			IntVector2 zone_pos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
			zone = this->world->GetZone(zone_pos);
		} else {
			zone = this->zone;
		}

		// if zone does not exist return no position
		if (!zone) {
			return cubewg::kNoPosition;
		}

		cube::Block* blocc;
		int base_z = zone->fields->base_z;
		bool had_block = false;

		for (int zo = 0; zo < 64; zo++) {
			blocc = zone->GetBlock(IntVector3(32, 32, base_z + zo));

			if (blocc) {
				if (blocc->type == cube::Block::Air || blocc->type == cube::Block::Water || blocc->type == cube::Block::Lava) {
					return (had_block || !require_surface) ? base_z + zo : cubewg::kNoPosition;
				}

				had_block = true;
			} else if (had_block) {
				return base_z + zo;
			}
		}

		return require_surface ? cubewg::kNoPosition : base_z;
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

			IntVector2 zone_pos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
			to_remesh.insert(this->world->GetZone(zone_pos));

			LongVector3 local_block_pos = LongVector3(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);

			if (local_block_pos.x == 0) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zone_pos.x - 1, zone_pos.y));
				if (zone) to_remesh.insert(zone);
			} else if (local_block_pos.x == cube::BLOCKS_PER_ZONE - 1) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zone_pos.x + 1, zone_pos.y));
				if (zone) to_remesh.insert(zone);
			}

			if (local_block_pos.y == 0) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zone_pos.x, zone_pos.y - 1));
				if (zone) to_remesh.insert(zone);
			} else if (local_block_pos.y == cube::BLOCKS_PER_ZONE - 1) {
				cube::Zone* zone = this->world->GetZone(IntVector2(zone_pos.x, zone_pos.y + 1));
				if (zone) to_remesh.insert(zone);
			}
		}
		else {
			this->zone->SetBlock(ToLocalBlockPos(block_pos), block, false);
			to_remesh.insert(this->zone);
			cube::World* world = this->zone->world;
			IntVector2 zone_pos = this->zone->position;

			// make sure neighbouring zones are refreshed if they are loaded

			if (block_pos.x == 0) {
				cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x - 1, zone_pos.y));
				if (zone) to_remesh.insert(zone);
			} else if (block_pos.x == cube::BLOCKS_PER_ZONE - 1) {
				cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x + 1, zone_pos.y));
				if (zone) to_remesh.insert(zone);
			}

			if (block_pos.y == 0) {
				cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x, zone_pos.y - 1));
				if (zone) to_remesh.insert(zone);
			} else if (block_pos.y == cube::BLOCKS_PER_ZONE - 1) {
				cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x, zone_pos.y + 1));
				if (zone) to_remesh.insert(zone);
			}
		}
	}
}
