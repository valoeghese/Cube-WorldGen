#include "DebugTree.h"

cubewg::DebugTree::DebugTree()
{
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
}

int cubewg::DebugTree::GenerateAt(WorldRegion& region, const IntVector3& origin, std::set<cube::Zone*>& to_remesh)
{
	const int kHeight = 10;
	const int x = origin.x;
	const int y = origin.y;
	const int z = origin.z;

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

	return 0;
}

bool cubewg::DebugTree::Generate(WorldRegion & region, const IntVector2 & zone_position, std::set<cube::Zone*>& to_remesh)
{
	// don't generate in practise.
	return false;
}
