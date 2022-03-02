#include "main.h"

#include "JitteredGrid.h"

#define LF L"\n";

// Includes for the self written hooks.
// For example: #include "src/hooks/a_hook.h" 

namespace cubewg {
	JitteredGrid citiesGrid;

	cube::Block blue_leaves;
	cube::Block log;
	cube::Block city_wall;
	cube::Block debug_zone_border;

	cube::Block Block(const int r, const int g, const int b, const cube::Block::Type type = cube::Block::Solid, const bool breakable = false) {
		cube::Block result;
		result.red = r;
		result.green = g;
		result.blue = b;
		result.type = type;
		result.breakable = breakable;
		return result; // yeah this copies the entire struct but does it really matter
	}

	/* Mod class containing all the functions for the mod.
	*/
	class WorldGenMod : GenericMod {
		void GenerateTree(WorldRegion& region, int x, int y, int z, std::set<cube::Zone*>& to_remesh) {
			const int kHeight = 10;

			// generate trunk
			for (int i = 0; i < kHeight - 4; i++) {
				region.SetBlock(LongVector3(x, y, z + i), log, to_remesh);
			}

			// generate leaves
			for (int i = kHeight - 4; i < kHeight; i++) {
				int total_z = z + i;

				if (i == kHeight - 1) {
					for (int xo = -1; xo <= 1; xo++) {
						int total_x = x + xo;

						for (int yo = -1; yo <= 1; yo++) {
							int total_y = y + yo;

							if (xo == 0 || yo == 0) { // not corners
								region.SetBlock(LongVector3(total_x, total_y, total_z), blue_leaves, to_remesh);
							}
						}
					}
				} else {
					for (int xo = -2; xo <= 2; xo++) {
						int totalX = x + xo;

						for (int yo = -2; yo <= 2; yo++) {
							int totalY = y + yo;

							region.SetBlock(LongVector3(totalX, totalY, total_z), blue_leaves, to_remesh);
						}
					}
				}
			}
		}

		LongVector3 BlockFromDots(LongVector3 dots) {
			return LongVector3
			(
				pydiv(dots.x, cube::DOTS_PER_BLOCK),
				pydiv(dots.y, cube::DOTS_PER_BLOCK),
				pydiv(dots.z, cube::DOTS_PER_BLOCK)
			);
		}

		/* Hook for the chat function. Triggers when a user sends something in the chat.
		 * @param	{std::wstring*} message
		 * @return	{int}
		*/
		virtual int OnChat(std::wstring* message) override {
			bool allpos = *message == L".pos";

			if (*message == L".gentree") {
				try {
					LongVector3 playerPos = BlockFromDots(cube::GetGame()->GetPlayer()->entity_data.position);
					std::set<cube::Zone*> chunks_to_remesh;

					WorldRegion region = WorldRegion(cube::GetGame()->world);

					GenerateTree(region, playerPos.x, playerPos.y, playerPos.z, chunks_to_remesh);

					std::wstring w = std::to_wstring(chunks_to_remesh.size()) + LF;
					cube::GetGame()->PrintMessage((L"Remeshing N Chunks: " + w).c_str());

					std::set<cube::Zone*>::iterator itr;

					for (itr = chunks_to_remesh.begin(); itr != chunks_to_remesh.end(); itr++) {
						cube::Zone* zone = *itr;
						zone->chunk.Remesh();
					}

					return 1;
				}
				catch (std::exception& e) { // just in case but I feel that the game will just shit itself anyway
					std::string s = std::string(e.what());
					std::wstring ws(s.begin(), s.end());
					cube::GetGame()->PrintMessage(ws.c_str());
				}
			} else if (*message == L".height") {
				// get the world the player is in
				cube::World* world= cube::GetGame()->world;
				WorldRegion region(world);
				LongVector3 position = BlockFromDots(cube::GetGame()->GetPlayer()->entity_data.position);

				int height = region.GetHeight(LongVector2(position.x, position.y));
				int height_surface_required = region.GetHeight(LongVector2(position.x, position.y), true);

				std::wstring feedback = L"Height at your current position is " + std::to_wstring(height) + L" (Surface Required: " + std::to_wstring(height_surface_required) + L")" + LF;
				cube::GetGame()->PrintMessage(feedback.c_str());

				return 1;
			} else if (allpos || message->substr(0, 5) == L".pos ") {
				cube::Creature* player = cube::GetGame()->GetPlayer();
				LongVector3 position = BlockFromDots(player->entity_data.position);

				std::wstring feedback;

				if (allpos || *message == L".pos block") {
					feedback = L"Player Block Position is " + std::to_wstring(position.x) + L", " + std::to_wstring(position.y) + L", " + std::to_wstring(position.z) + LF;
					cube::GetGame()->PrintMessage(feedback.c_str());
				}

				if (allpos || *message == L".pos local") {
					feedback = L"Player Local Block Position is " + std::to_wstring(pymod(position.x, cube::BLOCKS_PER_ZONE)) + L", " + std::to_wstring(pymod(position.y, cube::BLOCKS_PER_ZONE)) + LF;
					cube::GetGame()->PrintMessage(feedback.c_str());
				}

				if (allpos || *message == L".pos basez") {
					WorldRegion region(cube::GetGame()->world);

					feedback = L"The current column's base Z is " + std::to_wstring(region.GetBaseZ(LongVector2(position.x, position.y))) + LF;
					cube::GetGame()->PrintMessage(feedback.c_str());
				}

				if (allpos || *message == L".pos worley") {
					cube::Zone* zone = cube::GetGame()->world->GetCurrentZone();
					long long int x = pymod(position.x, cube::BLOCKS_PER_ZONE);
					long long int y = pymod(position.y, cube::BLOCKS_PER_ZONE);

					double sample_x = (zone->position.x * cube::BLOCKS_PER_ZONE - x) * 0.001;
					double sample_y = (zone->position.y * cube::BLOCKS_PER_ZONE - y) * 0.001;

					double worley = citiesGrid.Worley(sample_x, sample_y);
					unsigned char grey = worley > 1 ? 255 : (unsigned char)(worley * 255.0);

					feedback = L"The current column's worley is " + std::to_wstring(worley) + L" (greyness: " + std::to_wstring(grey)
						+ L", Sample: [" + std::to_wstring(sample_x) + L", " + std::to_wstring(sample_y) + L"])" + LF;
					cube::GetGame()->PrintMessage(feedback.c_str());
				}

				if (allpos || *message == L".pos read") {
					cube::Block* blocc = cube::GetGame()->world->GetBlock(position);

					if (blocc) {
						feedback = L"Block at this position is type " + std::to_wstring(blocc->type) + LF;
					} else {
						feedback = L"No Block is at this position.\n";
					}

					cube::GetGame()->PrintMessage(feedback.c_str());
				}

				return 1;
			}
			return 0;
		}

		/* Function hook that gets called every game tick.
		 * @param	{cube::Game*} game
		 * @return	{void}
		*/
		virtual void OnGameTick(cube::Game* game) override {
			return;
		}

		/* Function hook that gets called on intialization of cubeworld.
		 * [Note]:	cube::GetGame() is not yet available here!!
		 * @return	{void}
		*/
		virtual void Initialize() override {
			WorldRegion::Initialise();

			blue_leaves.red = 0;
			blue_leaves.green = 30;
			blue_leaves.blue = 140;
			blue_leaves.type = cube::Block::Leaves;
			blue_leaves.breakable = true;

			log.red = 130;
			log.green = 90;
			log.blue = 0;
			log.type = cube::Block::Tree;
			log.breakable = false;

			city_wall.red = 130;
			city_wall.green = 150;
			city_wall.blue = 160;
			city_wall.type = cube::Block::Solid;
			city_wall.breakable = false;

			debug_zone_border = Block(255, 255, 0, cube::Block::Solid, true);
			return;
		}

		/* Function hook that gets called when a Zone is generated.
		*/
		virtual void OnZoneGenerated(cube::Zone* zone) override {
			WorldRegion region = WorldRegion(zone);

			cube::Block* blocc;
			bool hadblocc = false;
			std::set<cube::Zone*> to_remesh;

			WorldRegion::PasteZone(zone, to_remesh);

			// generate city walls
			for (int x = 0; x < cube::BLOCKS_PER_ZONE; x++) {
				for (int y = 0; y < cube::BLOCKS_PER_ZONE; y++) {
					double worley = citiesGrid.Worley((zone->position.x * cube::BLOCKS_PER_ZONE - x) * 0.001, (zone->position.y * cube::BLOCKS_PER_ZONE - y) * 0.001);

					int height = region.GetHeight(LongVector2(x, y), true);

					if (height != kNoPosition) {
						unsigned char grey = worley > 1 ? 255 : (unsigned char) (worley * 255.0);
						cube::Block noise_block = Block(grey, grey, grey);
						region.SetBlock(LongVector3(x, y, height), noise_block, to_remesh);

						if (x == 0 || y == 0) {
							region.SetBlock(LongVector3(x, y, height + 1), debug_zone_border, to_remesh);
						}
					}

					/*if (worley <= 0.1 && worley >= 0.08) {
						int height = region.GetHeight(LongVector2(x, y), true);

						if (height != kNoPosition) {
							for (int zo = 0; zo < 10; zo++) {
								region.SetBlock(LongVector3(x, y, height + zo), city_wall, to_remesh);
							}
						}
					}*/
				}
			}

			for (cube::Zone* zone : to_remesh) {
				zone->chunk.Remesh();
			}
		}

		virtual void OnZoneDestroy(cube::Zone* zone) override {
			WorldRegion::CleanUpBuffers(zone->position);
		}
	};
}

// Export of the mod created in this file, so that the modloader can see and use it.
EXPORT cubewg::WorldGenMod* MakeMod() {
	return new cubewg::WorldGenMod();
}
