#include "main.h"

// includes from Worldgen Mod
#include "WorldRegion.h"
#include "JitteredGrid.h"
#include "City.h"

#define LF L"\n";

// Includes for the self written hooks.
// For example: #include "src/hooks/a_hook.h" 

namespace cubewg {
	/* Mod class containing all the functions for the mod.
	*/
	class WorldGenMod : GenericMod {
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

			if (message->substr(0, 9) == L".generate") {
				try {
					LongVector3 playerPos = BlockFromDots(cube::GetGame()->GetPlayer()->entity_data.position);
					std::set<cube::Zone*> chunks_to_remesh;

					int feedback = WorldRegion::GenerateStructureAt(message->substr(9), playerPos, chunks_to_remesh);

					if (feedback) {
						cube::GetGame()->PrintMessage((L"Error Code: " + std::to_wstring(feedback)).c_str());
					}

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

				int height_motion_blocking = region.GetHeight(LongVector2(position.x, position.y), Heightmap::MOTION_BLOCKING);
				int height_world_surface = region.GetHeight(LongVector2(position.x, position.y), Heightmap::WORLD_SURFACE);
				int height_ocean_floor = region.GetHeight(LongVector2(position.x, position.y), Heightmap::OCEAN_FLOOR);

				std::wstring feedback = L"Heightmaps at your current position are (MB: " + std::to_wstring(height_motion_blocking)
					+ L", WS: " + std::to_wstring(height_world_surface)
					+ L", OF: " + std::to_wstring(height_ocean_floor) + L")" + LF;
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

			City* city = new City;
			WorldRegion::AddStructure(L"city", city);
			return;
		}

		/* Function hook that gets called when a Zone is generated.
		*/
		virtual void OnZoneGenerated(cube::Zone* zone) override {
			WorldRegion region = WorldRegion(zone);

			std::set<cube::Zone*> to_remesh;

			WorldRegion::GenerateInZone(zone, to_remesh);

			for (cube::Zone* zone : to_remesh) {
				zone->chunk.Remesh();
			}

			/*for (int x = 0; x < 64; x++) {
				for (int y = 0; y < 64; y++) {
					int field_index = x * cube::BLOCKS_PER_ZONE + y;
					cube::Field* field = &zone->fields[field_index];
					field->base_z = 10;
				}
			}
			zone->chunk.Remesh();*/
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
