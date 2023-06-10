#include "WorldRegion.h"

#include <mutex>
#include <list>

namespace std {
	template <>
	struct hash<Vector3<int>> {
		std::size_t operator()(const Vector3<int>& k) const {
			uint64_t x = (uint32_t)k.x;
			uint64_t y = (uint32_t)k.y;
			uint64_t vec2key = x | (y << 32);

			uint64_t z = (uint32_t)k.z;
			vec2key = vec2key * 31L + z;

			// Call into the MSVC-STL FNV-1a std::hash function.
			return std::hash<uint64_t>()(vec2key);
		}
	};
}

namespace cubewg {
	// the list of stuff to generate! We use a linked list rather than std::vector to ensure fast addition and iteration. And we don't need random access.
	std::list<Structure*>* structures;
	std::unordered_map<std::wstring, Structure*>* named_structures;

	// internal header stuff
	void SetBlockInZone(cube::Zone* zone, IntVector3 local_block_pos, cube::Block block, std::set<cube::Zone*>& to_remesh);

	// structs
	struct ZoneBuffer {
		std::unordered_map<IntVector3, cube::Block>* to_paste = nullptr;
	};

	struct ZoneBufferArr8 {
		ZoneBuffer neighbours[8];
	};

	// Map from the owner zone to buffers to paste in the region
	std::unordered_map<IntVector2, ZoneBufferArr8>* zoneBuffers;

	void WorldRegion::Initialise() {
		// iirc there were runtime crashes if I didn't delay initialisation. Hence, pointers.
		zoneBuffers = new std::unordered_map<IntVector2, ZoneBufferArr8>;
		structures = new std::list<Structure*>;
		named_structures = new std::unordered_map<std::wstring, Structure*>;
	}

	/* Get the location in an array of buffers for the x_dif and y_dif (dx and dy from the parent to the zone to write in)
	*/
	int BufferArrLoc(int x_dif, int y_dif) {
		switch (y_dif) {
		case -1:
			return 1 + x_dif; // 0, 1, 2
		case 1:
			return 3 + 1 + x_dif; // 3, 4, 5
		case 0:
			return x_dif > 0 ? 6 : 7; // 6, 7
		default:
			throw std::invalid_argument("Bad Coordinates");
		}
	}

	// Cleans up the memory here
	// Please don't have memory leaks please don't have memory leaks
	// I hate memory management

	std::mutex mut;

	void WorldRegion::AddStructure(std::wstring id, cubewg::Structure* structure) {
		(*named_structures)[id] = structure;
		structures->push_back(structure);
	}

	void WorldRegion::CleanUpBuffers(IntVector2 zone_pos) {
		if (!zoneBuffers) return;

		mut.lock(); // lock

		std::unordered_map<IntVector2, ZoneBufferArr8>::iterator bufs = zoneBuffers->find(zone_pos);
		
		if (bufs != zoneBuffers->end()) {
			ZoneBufferArr8& buffer_collection = bufs->second;
			
			for (ZoneBuffer& zone_buffer : buffer_collection.neighbours) {
				if (zone_buffer.to_paste) {
					delete zone_buffer.to_paste;
					zone_buffer.to_paste = nullptr;
				}
			}

			zoneBuffers->erase(zone_pos);
		}

		mut.unlock();
	}

	void WorldRegion::GenerateInZone(cube::Zone* zone, std::set<cube::Zone*>& to_remesh) {
		if (!zoneBuffers) return;

		mut.lock(); // lock

		int base_x = zone->position.x;
		int base_y = zone->position.y;

		// search around it in a square for buffers situated in this zone
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0) continue; // cannot buffer into self

				IntVector2 search_location(base_x + dx, base_y + dy);

				std::unordered_map<IntVector2, ZoneBufferArr8>::iterator bufs = zoneBuffers->find(search_location);

				if (bufs != zoneBuffers->end()) {
					ZoneBufferArr8& buffer_collection = bufs->second;

					// reverse of dx and dy to get the relative coords of this zone from the buffer's parent zone
					int index = BufferArrLoc(-dx, -dy);
					//cube::GetGame()->PrintMessage(L"Gonna look in index ");
					//cube::GetGame()->PrintMessage(std::to_wstring(index).c_str());
					//cube::GetGame()->PrintMessage(L"\n");
					std::unordered_map<IntVector3, cube::Block>* to_paste = buffer_collection.neighbours[index].to_paste;

					if (to_paste) {
						//cube::GetGame()->PrintMessage(L"Placing ");
						//cube::GetGame()->PrintMessage(std::to_wstring(to_paste->size()).c_str());
						//cube::GetGame()->PrintMessage(L" Blocks.\n");
						for (auto it = to_paste->begin(); it != to_paste->end(); it++) {
							SetBlockInZone(zone, it->first, it->second, to_remesh);
						}

						delete to_paste;
						buffer_collection.neighbours[index].to_paste = nullptr;
					}
				}
			}
		}

		mut.unlock();

		WorldRegion region(zone);

		for (Structure* structure : *structures) {
			structure->Generate(region, zone->position, to_remesh);
		}
	}

	void SetBlockInBuffer(cube::Zone* parent, int dx, int dy, IntVector3 local_block_pos, cube::Block block) {
		if (!zoneBuffers) return;

		std::unordered_map<IntVector2, ZoneBufferArr8>::iterator bufs = zoneBuffers->find(parent->position);
		int index = BufferArrLoc(dx, dy);

		// I still hate memory management
		if (bufs != zoneBuffers->end()) {
			ZoneBufferArr8& buffer_collection = bufs->second;

			// initialise if not yet
			if (!buffer_collection.neighbours[index].to_paste) {
				buffer_collection.neighbours[index].to_paste = new std::unordered_map<IntVector3, cube::Block>;
			}

			// add the block
			(*buffer_collection.neighbours[index].to_paste)[local_block_pos] = block;
		} else {
			// create value
			ZoneBufferArr8 new_val;
			new_val.neighbours[index].to_paste = new std::unordered_map<IntVector3, cube::Block>;
			(*new_val.neighbours[index].to_paste)[local_block_pos] = block;

			(*zoneBuffers)[parent->position] = new_val;
		}
	}

	// conversions

	// modulo x and y by blocks in each zone (64x64)
	IntVector3 ToLocalBlockPos(LongVector3 block_pos) {
		return IntVector3(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);
	}

	IntVector2 ToLocalBlockPos(LongVector2 block_pos) {
		return IntVector2(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE));
	}

	// if you can assume it is already

	IntVector3 AsLocalBlockPos(LongVector3 block_pos, const bool restrict_to_zone = true) {
		if (restrict_to_zone && (block_pos.x < 0 || block_pos.y < 0 || block_pos.y >= cube::BLOCKS_PER_ZONE || block_pos.x >= cube::BLOCKS_PER_ZONE)) {
			throw std::invalid_argument("Block Positions are outside of the restricted zone.");
		}

		return IntVector3(block_pos.x, block_pos.y, block_pos.z);
	}

	IntVector2 AsLocalBlockPos(LongVector2 block_pos, const bool restrict_to_zone = true) {
		if (restrict_to_zone && (block_pos.x < 0 || block_pos.y < 0 || block_pos.y >= cube::BLOCKS_PER_ZONE || block_pos.x >= cube::BLOCKS_PER_ZONE)) {
			throw std::invalid_argument("Block Positions are outside of the restricted zone.");
		}

		return IntVector2(block_pos.x, block_pos.y);
	}

	// instance methods for WorldRegion

	WorldRegion::WorldRegion(cube::World* world) {
		this->world = world;
		this->zone = nullptr;
	}

	WorldRegion::WorldRegion(cube::Zone* zone) {
		this->world = nullptr;
		this->zone = zone;
	}

	WorldRegion::~WorldRegion() {
		// NO-OP
	}

	cube::Block* WorldRegion::GetBlock(LongVector3 block_pos) {
		if (this->world) {
			return this->world->GetBlock(block_pos);
		} else {
			return this->zone->GetBlock(AsLocalBlockPos(block_pos));
		}
	}

	int WorldRegion::GetBaseZ(LongVector2 block_pos) {
		cube::Zone* zone;
		IntVector2 local_block_pos;

		if (this->world) {
			IntVector2 zone_pos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
			zone = this->world->GetZone(zone_pos);
			local_block_pos = ToLocalBlockPos(block_pos);
		} else {
			zone = this->zone;
			local_block_pos = AsLocalBlockPos(block_pos);
		}

		// if zone does not exist return no position
		if (!zone) {
			return cubewg::kNoPosition;
		}

		int field_index = local_block_pos.x * cube::BLOCKS_PER_ZONE + local_block_pos.y;
		cube::Field* field = &zone->fields[field_index];
		return field->base_z;
	}

	int WorldRegion::GetHeight(LongVector2 block_pos, const Heightmap heightmap) {
		cube::Zone* zone;
		IntVector2 local_block_pos;

		if (this->world) {
			IntVector2 zone_pos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
			zone = this->world->GetZone(zone_pos);
			local_block_pos = ToLocalBlockPos(block_pos);
		} else {
			zone = this->zone;
			local_block_pos = AsLocalBlockPos(block_pos);
		}

		// if zone does not exist return no position
		if (!zone) {
			return cubewg::kNoPosition;
		}
		
		int field_index = local_block_pos.x * cube::BLOCKS_PER_ZONE + local_block_pos.y;
		cube::Field* field = &zone->fields[field_index];
		int base_z = field->base_z;

		cube::Block* blocc;

		switch (heightmap) {
		case Heightmap::WORLD_SURFACE:
			// First block with air above it is world surface.
			// Start at 1 as cannot return below base_z
			for (int zo = 1; zo < 64; zo++) {
				blocc = zone->GetBlock(IntVector3(local_block_pos.x, local_block_pos.y, base_z + zo));

				if (!blocc || blocc->type == cube::Block::Air) {
					return base_z + zo - 1;
				}
			}

			// No block found
			// Assume no position.
			return cubewg::kNoPosition;
		case Heightmap::MOTION_BLOCKING:
		case Heightmap::OCEAN_FLOOR:
			// Search from top to bottom.
			for (int zo = 63; zo >= 0; zo--) {
				blocc = zone->GetBlock(IntVector3(local_block_pos.x, local_block_pos.y, base_z + zo));

				if (blocc) {
					if (blocc->type != cube::Block::Air) {
						switch (heightmap) {
						case Heightmap::MOTION_BLOCKING:
							return base_z + zo;
						case Heightmap::OCEAN_FLOOR:
							// Ocean floor goes through liquids.
							if (blocc->type != cube::Block::Water && blocc->type != cube::Block::Lava) {
								return base_z + zo;
							}

							break;
						}
					}
				}
			}

			// No block found
			// Assume heightmap is at the base position.
			return base_z;
		default:
			return cubewg::kNoPosition;
		}
	}

	void SetBlockInZone(cube::Zone* zone, IntVector3 local_block_pos, cube::Block block, std::set<cube::Zone*>& to_remesh) {
		zone->SetBlock(local_block_pos, block, false);
		to_remesh.insert(zone);

		// make sure neighbouring zones are refreshed if they are loaded
		cube::World* world = zone->world;
		IntVector2 zone_pos = zone->position;

		if (local_block_pos.x == 0) {
			cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x - 1, zone_pos.y));
			if (zone) to_remesh.insert(zone);
		} else if (local_block_pos.x == cube::BLOCKS_PER_ZONE - 1) {
			cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x + 1, zone_pos.y));
			if (zone) to_remesh.insert(zone);
		}

		if (local_block_pos.y == 0) {
			cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x, zone_pos.y - 1));
			if (zone) to_remesh.insert(zone);
		} else if (local_block_pos.y == cube::BLOCKS_PER_ZONE - 1) {
			cube::Zone* zone = world->GetZone(IntVector2(zone_pos.x, zone_pos.y + 1));
			if (zone) to_remesh.insert(zone);
		}
	}

	void WorldRegion::SetBlock(LongVector3 block_pos, cube::Block block, std::set<cube::Zone*>& to_remesh) {
		if (this->world) {
			this->world->SetBlock(block_pos, block, false);

			IntVector2 zone_pos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
			to_remesh.insert(this->world->GetZone(zone_pos));

			IntVector3 local_block_pos = ToLocalBlockPos(block_pos);

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
			// handle generation into the neighbouring 8 zones via buffers
			// coordinates should be in zone coords
			int dx = 0;
			int dy = 0;

			if (block_pos.x < 0) {
				dx = -1;
			}
			else if (block_pos.x >= cube::BLOCKS_PER_ZONE) {
				dx = 1;
			}

			if (block_pos.y < 0) {
				dy = -1;
			} else if (block_pos.y >= cube::BLOCKS_PER_ZONE) {
				dy = 1;
			}

			if (dx == 0 && dy == 0) { // if they're within our zone just use it
				SetBlockInZone(this->zone, AsLocalBlockPos(block_pos), block, to_remesh);
			}
			else {
				IntVector2 zone_pos = this->zone->position;
				mut.lock();
				cube::Zone* zone = this->zone->world->GetZone(zone_pos.x + dx, zone_pos.y + dy);

				if (zone) {
					mut.unlock();
					SetBlockInZone(zone, ToLocalBlockPos(block_pos), block, to_remesh);
				} else {
					SetBlockInBuffer(this->zone, dx, dy, ToLocalBlockPos(block_pos), block);
					mut.unlock();
				}
			}
		}
	}

	cube::Block BlockOf(const int r, const int g, const int b, const cube::Block::Type type, const bool breakable) {
		cube::Block result;
		result.red = r;
		result.green = g;
		result.blue = b;
		result.type = type;
		result.breakable = breakable;
		return result; // yeah this copies the entire struct but does it really matter
	}
}
