// Based off of code I wrote here https://github.com/valoeghese/2fc0f18/blob/master/src/main/java/tk/valoeghese/fc0/world/kingdom/Voronoi.java

#include "JitteredGrid.h"

#include <limits>
#include <cmath>

const int64_t kMaxInt64 = std::numeric_limits<int64_t>::max();

// constructors

cubewg::JitteredGrid::JitteredGrid(const int64_t seed) {
	this->seed = seed;
	this->relaxation = 0;
	this->scale = 1;
}

cubewg::JitteredGrid::JitteredGrid(const int64_t seed, const double relaxation) {
	this->seed = seed;
	this->relaxation = relaxation;
	this->scale = 1;
}

cubewg::JitteredGrid::JitteredGrid(const int64_t seed, const double relaxation, const double scale) {
	this->seed = seed;
	this->relaxation = relaxation;
	this->scale = scale;
}

// methods

int64_t Random(int64_t seed, int64_t x, int64_t y) {
	// constants from MMIX, however the exact rng implementation differs
	seed *= seed * 6364136223846793005 + 1442695040888963407;
	seed += x;
	seed *= seed * 6364136223846793005 + 1442695040888963407;
	seed += y;
	seed *= seed * 6364136223846793005 + 1442695040888963407;
	seed += x;
	seed *= seed * 6364136223846793005 + 1442695040888963407;
	seed += y;
	return seed;
}

double RandomDouble(int64_t seed, int64_t x, int64_t y) {
	return (double)Random(seed, x, y) / (double)kMaxInt64;
}

double SqrDist(double x0, double y0, double x1, double y1) {
	double dx = x1 - x0;
	double dy = y1 - y0;
	return dx * dx + dy * dy;
}

cubewg::JitteredPoint cubewg::JitteredGrid::SampleGrid(int64_t grid_x, int64_t grid_y) {
	double unrelaxation = 1.0 - this->relaxation; // the "opposite" of the relaxation in weighting the values
	return cubewg::JitteredPoint(
		this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed, grid_x, grid_y),
		this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed + 1, grid_x, grid_y),
		Random(this->seed + 2, grid_x, grid_y));
}

cubewg::JitteredPoint cubewg::JitteredGrid::SampleVoronoi(double x, double y) {
	// Scale down inputs
	x /= this->scale;
	y /= this->scale;

	double unrelaxation = 1.0 - this->relaxation;

	// coordinates of the grid area in the centre of the search. I.e. the grid area the point is actually in.
	const uint64_t cgrid_x = (uint64_t)std::floor(x);
	const uint64_t cgrid_y = (uint64_t)std::floor(y);

	double result_x = 0;
	double result_y = 0;
	uint64_t result_grid_x = 0;
	uint64_t result_grid_y = 0;
	double result_dist = 1000.0; // 1000 is an insanely high value we can't possibly get, so it's used as a placeholder

	for (int xo = -1; xo <= 1; xo++) {
		int grid_x = cgrid_x + xo;

		for (int yo = -1; yo <= 1; yo++) {
			int grid_y = cgrid_y + yo;

			double point_x = grid_x + this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed, grid_x, grid_y);
			double point_y = grid_y + this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed + 1, grid_x, grid_y);
			double point_dist = SqrDist(x, y, point_x, point_y);

			if (point_dist < result_dist) {
				result_x = point_x;
				result_y = point_y;
				result_grid_x = grid_x;
				result_grid_y = grid_y;
				result_dist = point_dist;
			}
		}
	}

	// Scale up output position
	return JitteredPoint(result_x * this->scale, result_y * this->scale, Random(this->seed + 2, result_grid_x, result_grid_y));
}

double cubewg::JitteredGrid::Worley(double x, double y) {
	// Scale down inputs
	x /= this->scale;
	y /= this->scale;

	double unrelaxation = 1.0 - this->relaxation;

	// coordinates of the grid area in the centre of the search. I.e. the grid area the point is actually in.
	const uint64_t cgrid_x = (uint64_t)std::floor(x);
	const uint64_t cgrid_y = (uint64_t)std::floor(y);

	double result_dist = 1000.0; // 1000 is an insanely high value we can't possibly get, so it's used as a placeholder

	for (int xo = -2; xo <= 2; xo++) {
		int grid_x = cgrid_x + xo;

		for (int yo = -2; yo <= 2; yo++) {
			if (std::abs(xo) == 2 && std::abs(yo) == 2) continue;

			int grid_y = cgrid_y + yo;

			double point_x = grid_x + this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed, grid_x, grid_y);
			double point_y = grid_y + this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed + 1, grid_x, grid_y);
			double point_dist = SqrDist(x, y, point_x, point_y);

			if (point_dist < result_dist) {
				result_dist = point_dist;
			}
		}
	}

	// Scale up output
	// Uses squared distance so to scale up value a by factor k for expression a^2, we must multiply by square
	// (a * k) ^2 = a^2 * k^2
	return result_dist * (this->scale * this->scale);
}

double cubewg::JitteredGrid::Worley2(double x, double y) {
	// Scale down inputs
	x /= this->scale;
	y /= this->scale;

	double unrelaxation = 1.0 - this->relaxation;

	// coordinates of the grid area in the centre of the search. I.e. the grid area the point is actually in.
	const uint64_t cgrid_x = (uint64_t)std::floor(x);
	const uint64_t cgrid_y = (uint64_t)std::floor(y);

	double result_dist = 1000.0; // 1000 is an insanely high value we can't possibly get, so it's used as a placeholder
	double result_dist_2 = 1000.0; // Again, a placeholder high value. This will be the second closest distance.

	for (int xo = -2; xo <= 2; xo++) {
		int grid_x = cgrid_x + xo;

		for (int yo = -2; yo <= 2; yo++) {
			int grid_y = cgrid_y + yo;

			double point_x = grid_x + this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed, grid_x, grid_y);
			double point_y = grid_y + this->relaxation * 0.5 + unrelaxation * RandomDouble(this->seed + 1, grid_x, grid_y);
			double point_dist = SqrDist(x, y, point_x, point_y);

			if (point_dist <= result_dist) {
				result_dist_2 = result_dist;
				result_dist = point_dist;
			} else if (point_dist < result_dist_2) {
				result_dist_2 = point_dist;
			}
		}
	}

	double result = result_dist_2 - result_dist;

	// Scale up output
	// Uses squared distance so to scale up value a by factor k for expression a^2, we must multiply by square
	// (a * k) ^2 = a^2 * k^2
	return result * (this->scale * this->scale);
}