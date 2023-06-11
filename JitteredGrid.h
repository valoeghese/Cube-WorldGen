#pragma once

#include <cstdint>

using std::int64_t;

namespace cubewg {
	struct JitteredPoint {
		// the x position of this point
		double x;
		// this y position of this point
		double y;
		// an additional random number associated with this point
		int64_t data;

		JitteredPoint(double x, double y, int64_t data) {
			this->x = x;
			this->y = y;
			this->data = data;
		}
	};

	class JitteredGrid {
	private:
		int64_t seed;
		// How much to move points on the grid towards the average.
		double relaxation;
		// Scale of the grid. That is, how much to scale input/output.
		// Inputs are scaled DOWN by this amount and outputs are then scaled UP.
		double scale;
	public:
		JitteredGrid(const int64_t seed);
		JitteredGrid(const int64_t seed, const double relaxation);
		JitteredGrid(const int64_t seed, const double relaxation, const double scale);

		/* Samples the point on the jittered grid at the given coordinates. The jittered point is guaranteed to fall within this grid space.
		*/
		JitteredPoint SampleGrid(int64_t grid_x, int64_t grid_y);

		/* Samples the nearest jittered point to the given coordinates.
		*/
		JitteredPoint FindNearestPoint(double x, double y);

		/* Samples cellular noise using the points on this jittered grid. Uses euclidean squared distance.
		*/
		double SqrDist2Nearest(double x, double y);

		/* Samples 'd2-d1' type cellular noise using the points on this jittered grid. Uses euclidean squared distance.
		*/
		double Worley2(double x, double y);
	};
}
