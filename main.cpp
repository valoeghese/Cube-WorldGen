#include "main.h"

#define LF L"\n";

/*
typedef struct {
	int x;
	int y;
} vec2;

bool operator==(const vec2& lhs, const vec2& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator<(const vec2& lhs, const vec2& rhs) {
	return lhs.x < rhs.x || lhs.y < rhs.y;
}

bool operator>(const vec2& lhs, const vec2& rhs) {
	return lhs.x > rhs.x || lhs.y > rhs.y;
}

// The specialized hash function for `unordered_map` keys
struct vec2_hash_fn {
	std::size_t operator() (const vec2& node) const {
		std::size_t h1 = std::hash<int>()(node.x);
		std::size_t h2 = std::hash<int>()(node.y);

		return h1 ^ h2;
	}
};
*/

//std::set<vec2>* wtfset;

// Includes for the self written hooks.
// For example: #include "src/hooks/a_hook.h" 

/* Mod class containing all the functions for the mod.
*/
class WorldGenMod : GenericMod {
	void GenerateTree(cubewg::WorldRegion& region, int x, int y, int z, std::set<cube::Zone*>& to_remesh) {
		const int kHeight = 10;

		// generate trunk
		for (int i = 0; i < kHeight - 4; i++) {
			region.SetBlock(LongVector3(x, y, z + i), 130, 90, 0, cube::Block::Solid, to_remesh);
		}

		// generate leaves
		for (int i = kHeight - 4; i < kHeight; i++) {
			int totalZ = z + i;

			if (i == kHeight - 1) {
				for (int xo = -1; xo <= 1; xo++) {
					int totalX = x + xo;

					for (int yo = -1; yo <= 1; yo++) {
						int totalY = y + yo;

						if (xo == 0 || yo == 0) { // not corners
							region.SetBlock(LongVector3(totalX, totalY, totalZ), 0, 0, 140, cube::Block::Solid, to_remesh);
						}
					}
				}
			} else {
				for (int xo = -2; xo <= 2; xo++) {
					int totalX = x + xo;

					for (int yo = -2; yo <= 2; yo++) {
						int totalY = y + yo;

						region.SetBlock(LongVector3(totalX, totalY, totalZ), 0, 0, 140, cube::Block::Solid, to_remesh);
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
		if (*message == std::wstring(L"gentree")) {
			try {
				LongVector3 playerPos = BlockFromDots(cube::GetGame()->GetPlayer()->entity_data.position);
				std::set<cube::Zone*> chunks_to_remesh;

				cubewg::WorldRegion region = cubewg::WorldRegion(cube::GetGame()->world);

				GenerateTree(region, playerPos.x, playerPos.y, playerPos.z, chunks_to_remesh);

				std::wstring w = std::to_wstring(chunks_to_remesh.size());
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
			catch (std::string& s) {
				std::wstring ws(s.begin(), s.end());
				cube::GetGame()->PrintMessage(ws.c_str());
			}
		}
		return 0;
	}

	/* Function hook that gets called every game tick.
	 * @param	{cube::Game*} game
	 * @return	{void}
	*/
	virtual void OnGameTick(cube::Game* game) override {
		/*cube::Zone* zone = game->world->GetCurrentZone();

		if (game->GetPlayer() && game->world && zone) {
			//cube::Creature* player = game->GetPlayer();
			//LongVector3 position = BlockFromDots(player->entity_data.position);

			//std::wstring message = L"Player Block Position is " + std::to_wstring(position.x) + L", " + std::to_wstring(position.y) + L", " + std::to_wstring(position.z) + L"\n";
			std::wstring message = L"Base Zone Position is " + std::to_wstring(zone->fields->base_z) + L"\n";
			game->PrintMessage(message.c_str());
		}*/
		return;
	}

	/* Function hook that gets called on intialization of cubeworld.
	 * [Note]:	cube::GetGame() is not yet available here!!
	 * @return	{void}
	*/
	virtual void Initialize() override {
		//wtfset = new std::set<vec2>;
		return;
	}

	/* Function hook that gets called when a Zone is generated.
	*/
	virtual void OnZoneGenerated(cube::Zone* zone) override {
		/*vec2 loc;
		loc.x = zone->position.x;
		loc.y = zone->position.y;

		if (wtfset->find(loc) != wtfset->end()) {
			cube::GetGame()->PrintMessage(L"Wtf");
		} else {
			wtfset->insert(loc);
		}*/
		cubewg::WorldRegion region = cubewg::WorldRegion(zone);

		int base_z = zone->fields->base_z;
		cube::Block* blocc;

		for (int zo = 0; zo < 64; zo++) {
			blocc = region.GetBlock(LongVector3(32, 32, base_z + zo));

			if (blocc) {
				if (blocc->type == cube::Block::Air) {
					std::set<cube::Zone*> to_remesh;

					GenerateTree(region, 32, 32, base_z + zo, to_remesh);

					std::wstring w = std::to_wstring(to_remesh.size()) + LF;
					cube::GetGame()->PrintMessage((L"Remeshing N Chunks due to Natural Tree Gen: " + w).c_str());
						
					for (cube::Zone* zone : to_remesh) {
						zone->chunk.Remesh();
					}

					break;
				}
			}
		}
	}
};

// Export of the mod created in this file, so that the modloader can see and use it.
EXPORT WorldGenMod* MakeMod() {
	return new WorldGenMod();
}
